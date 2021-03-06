// -*- C++ -*-

// GradNNetLayerModule.cc
//
// Copyright (C) 2005 Pascal Lamblin
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

/* *******************************************************
   * $Id: GradNNetLayerModule.cc,v 1.3 2006/01/18 04:04:06 lamblinp Exp $
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file GradNNetLayerModule.cc */


#include "GradNNetLayerModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    GradNNetLayerModule,
    "Affine transformation module, with stochastic gradient descent updates",
    "Neural Network layer, using stochastic gradient to update neuron weights\n"
    "       Output = weights * Input + bias\n"
    "Weights and bias are updated by online gradient descent, with learning\n"
    "rate possibly decreasing in 1/(1 + n_updates_done * decrease_constant).\n"
    "An L1 and L2 regularization penalty can be added to push weights to 0.\n"
    "Weights can be initialized to 0, to a given initial matrix, or randomly\n"
    "from a uniform distribution.\n"
    );

/////////////////////////
// GradNNetLayerModule //
/////////////////////////
GradNNetLayerModule::GradNNetLayerModule():
    start_learning_rate( .001 ),
    decrease_constant( 0. ),
    init_weights_random_scale( 1. ),
    L1_penalty_factor( 0. ),
    L2_penalty_factor( 0. ),
    step_number( 0 )
{}

///////////
// fprop //
///////////
void GradNNetLayerModule::fprop(const Vec& input, Vec& output) const
{
    PLASSERT_MSG( input.size() == input_size,
                  "input.size() should be equal to this->input_size" );

    output.resize( output_size );

    // Applies linear transformation
    for( int i=0 ; i<output_size ; i++ )
        output[i] = dot( weights(i), input ) + bias[i];
}

void GradNNetLayerModule::fprop(const Mat& inputs, Mat& outputs)
{
    PLASSERT( inputs.width() == input_size );
    int n = inputs.length();
    outputs.resize(n, output_size);
    productTranspose(outputs, inputs, weights);

    // Add bias.
    resizeOnes(n);
    externalProductAcc(outputs, ones, bias); // could be more efficient, but not critical
}

/////////////////
// bpropUpdate //
/////////////////
// We are not using blas routines anymore, because we would iterate several
// times over the weight matrix.
void GradNNetLayerModule::bpropUpdate(const Vec& input, const Vec& output,
                                      const Vec& output_gradient)
{
    PLASSERT_MSG( input.size() == input_size,
                  "input.size() should be equal to this->input_size" );
    PLASSERT_MSG( output.size() == output_size,
                  "output.size() should be equal to this->output_size" );
    PLASSERT_MSG( output_gradient.size() == output_size,
                  "output_gradient.size() should be equal to this->output_size"
                );

    learning_rate = start_learning_rate / (1+decrease_constant*step_number);

    for( int i=0; i<output_size; i++ )
    {
        real og_i = output_gradient[i];
        real* w_ = weights[i];

        real delta_L1 = learning_rate * L1_penalty_factor;
        real delta_L2 = learning_rate * L2_penalty_factor;
        if( delta_L2 > 1 )
            PLWARNING("GradNNetLayerModule::bpropUpdate:\n"
                      "learning rate = %f is too large!\n", learning_rate);

        real lr_og_i = learning_rate * og_i;
        bias[i] -= lr_og_i;

        for( int j=0; j<input_size; j++ )
        {
            if( delta_L2 > 0. )
                w_[j] *= (1 - delta_L2);

            w_[j] -= input[j] * lr_og_i;

            if( delta_L1 > 0. )
            {
                if( w_[j] > delta_L1 )
                    w_[j] -= delta_L1;
                else if( w_[j] < -delta_L1 )
                    w_[j] += delta_L1;
                else
                    w_[j] = 0.;
            }

        }
    }
    step_number++;
}


// Simply updates and propagates back gradient
void GradNNetLayerModule::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient,
                                      bool accumulate)
{
    PLASSERT_MSG( input.size() == input_size,
                  "input.size() should be equal to this->input_size" );
    PLASSERT_MSG( output.size() == output_size,
                  "output.size() should be equal to this->output_size" );
    PLASSERT_MSG( output_gradient.size() == output_size,
                  "output_gradient.size() should be equal to this->output_size"
                );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize( input_size );
        input_gradient.clear();
    }

    learning_rate = start_learning_rate / (1+decrease_constant*step_number);

    for( int i=0; i<output_size; i++ )
    {
        real og_i = output_gradient[i];
        real* w_ = weights[i];

        real delta_L1 = learning_rate * L1_penalty_factor;
        real delta_L2 = learning_rate * L2_penalty_factor;
        if( delta_L2 > 1 )
            PLWARNING("GradNNetLayerModule::bpropUpdate:\n"
                      "learning rate = %f is too large!\n", learning_rate);

        real lr_og_i = learning_rate * og_i;
        bias[i] -= lr_og_i;

        for( int j=0; j<input_size; j++ )
        {
            input_gradient[j] += w_[j] * og_i;

            if( delta_L2 > 0. )
                w_[j] *= (1 - delta_L2);

            w_[j] -= input[j] * lr_og_i;

            if( delta_L1 > 0. )
            {
                if( w_[j] > delta_L1 )
                    w_[j] -= delta_L1;
                else if( w_[j] < -delta_L1 )
                    w_[j] += delta_L1;
                else
                    w_[j] = 0.;
            }

        }
    }
    step_number++;
}

void GradNNetLayerModule::bpropUpdate(const Mat& inputs, const Mat& outputs,
        Mat& input_gradients,
        const Mat& output_gradients,
        bool accumulate)
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( outputs.width() == output_size );
    PLASSERT( output_gradients.width() == output_size );

    int n = inputs.length();

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == input_size &&
                input_gradients.length() == n,
                "Cannot resize input_gradients and accumulate into it" );
    }
    else
    {
        input_gradients.resize(n, input_size);
        input_gradients.fill(0);
    }

    learning_rate = start_learning_rate / (1+decrease_constant*step_number);
    real avg_lr = learning_rate / n; // To obtain an average on a mini-batch.

    // With L2 regularization, weights are scaled by a coefficient equal to
    // 1 - learning rate * penalty.
    real l2_scaling =
        L2_penalty_factor > 0 ? 1 - learning_rate * L2_penalty_factor
                              : 1;
    PLASSERT_MSG(l2_scaling > 0, "Learning rate too large");

    // Compute input gradient.
    productAcc(input_gradients, output_gradients, weights);

    // Update bias.
    resizeOnes(n);
    transposeProductScaleAcc(bias, output_gradients, ones, -avg_lr, real(1));

    // Update weights.
    transposeProductScaleAcc(weights, output_gradients, inputs,
                             -avg_lr, l2_scaling);

    // Apply L1 penalty if needed (note: this is not very efficient).
    if (L1_penalty_factor > 0) {
        real delta_L1 = learning_rate * L1_penalty_factor;
        for( int i=0; i<output_size; i++ )
        {
            real* w_ = weights[i];
            for( int j=0; j<input_size; j++ )
            {
                real& w_ij = w_[j];
                if( w_ij > delta_L1 )
                    w_ij -= delta_L1;
                else if( w_ij < -delta_L1 )
                    w_ij += delta_L1;
                else
                    w_ij = 0.;
            }
        }
    }
    step_number += n;
}


//////////////////
// bbpropUpdate //
//////////////////
void GradNNetLayerModule::bbpropUpdate(const Vec& input, const Vec& output,
                                       const Vec& output_gradient,
                                       const Vec& output_diag_hessian)
{
    PLASSERT_MSG( output_diag_hessian.size() == output_size,
                  "output_diag_hessian.size() should be equal to"
                  " this->output_size" );
    bpropUpdate( input, output, output_gradient );
}

/* This implementation is incorrect. Let the PLERROR defined in parent version
// Propagates back output_gradient and output_diag_hessian
void GradNNetLayerModule::bbpropUpdate(const Vec& input, const Vec& output,
                                       Vec&  input_gradient,
                                       const Vec& output_gradient,
                                       Vec&  input_diag_hessian,
                                       const Vec& output_diag_hessian,
                                       bool accumulate)
{
    bpropUpdate( input, output, input_gradient, output_gradient, accumulate );
}
*/

////////////
// forget //
////////////
// Forget the bias and reinitialize the weights
void GradNNetLayerModule::forget()
{
    learning_rate = start_learning_rate;
    step_number = 0;

    bias.resize( output_size );
    if( init_bias.size() > 0 )
    {
        if( init_bias.size() != output_size )
            PLERROR( "init_bias (%d) should have length equal to output_size (%d)",
                     init_bias.size(), output_size );
        bias << init_bias;
    }
    else
        bias.clear();

    weights.resize( output_size, input_size );
    if( init_weights.size() > 0 )
    {
        if( weights.length() != output_size || weights.width() != input_size )
            PLERROR( "weights (%d,%d) should have size equal to (output_size, input_size) (%d,%d)",
                     weights.length(), weights.width(),
                     output_size, input_size );

        weights << init_weights;
    }
    else if(init_weights_random_scale != 0. )
    {
        if( !random_gen )
        {
            PLWARNING( "GradNNetLayerModule: cannot forget() without"
                       " random_gen" );
            return;
        }
        real r = init_weights_random_scale / input_size;
        random_gen->fill_random_uniform(weights, -r, r);
    }
    else
        weights.clear();
}

void GradNNetLayerModule::setLearningRate( real dynamic_learning_rate )
{
    start_learning_rate = dynamic_learning_rate;
    step_number = 0;
    // learning_rate will automatically be set in bpropUpdate()
}

///////////
// build //
///////////
void GradNNetLayerModule::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GradNNetLayerModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(init_weights, copies);
    deepCopyField(init_bias,    copies);
    deepCopyField(weights,      copies);
    deepCopyField(bias,         copies);
    deepCopyField(ones,         copies);
}

////////////////////
// declareOptions //
////////////////////
void GradNNetLayerModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "start_learning_rate",
                  &GradNNetLayerModule::start_learning_rate,
                  OptionBase::buildoption,
                  "Learning-rate of stochastic gradient optimization");

    declareOption(ol, "decrease_constant",
                  &GradNNetLayerModule::decrease_constant,
                  OptionBase::buildoption,
                  "Decrease constant of stochastic gradient optimization");

    declareOption(ol, "init_weights", &GradNNetLayerModule::init_weights,
                  OptionBase::buildoption,
                  "Optional initial weights of the neurons (one row per neuron).\n"
                  "If not provided then weights are initialized according to a uniform\n"
                  "distribution (see init_weights_random_scale)\n");

    declareOption(ol, "init_bias", &GradNNetLayerModule::init_bias,
                  OptionBase::buildoption,
                  "Optional initial bias of the neurons. If not provided, they are set to 0.\n");

    declareOption(ol, "init_weights_random_scale",
                  &GradNNetLayerModule::init_weights_random_scale,
                  OptionBase::buildoption,
                  "If init_weights is not provided, the weights are initialized randomly\n"
                  "from a uniform in [-r,r], with r = init_weights_random_scale/input_size.\n"
                  "To clear the weights initially, just set this option to 0.");

    declareOption(ol, "L1_penalty_factor",
                  &GradNNetLayerModule::L1_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L1 regularization term, i.e.\n"
                  "minimize L1_penalty_factor * sum_{ij} |weights(i,j)| during training.\n");

    declareOption(ol, "L2_penalty_factor",
                  &GradNNetLayerModule::L2_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L2 regularization term, i.e.\n"
                  "minimize 0.5 * L2_penalty_factor * sum_{ij} weights(i,j)^2 during training.\n");


    declareOption(ol, "weights", &GradNNetLayerModule::weights,
                  OptionBase::learntoption,
                  "Input weights of the neurons (one row per neuron)");

    declareOption(ol, "bias", &GradNNetLayerModule::bias,
                  OptionBase::learntoption,
                  "Bias of the neurons");

    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void GradNNetLayerModule::build_()
{
    if( input_size < 0 ) // has not been initialized
        return;

    if( output_size < 0 )
        PLERROR("GradNNetLayerModule::build_: 'output_size' is < 0 (%i),\n"
                " you should set it to a positive integer (the number of"
                " neurons).\n", output_size);

    if( weights.length() != output_size
        || weights.width() != input_size
        || bias.size() != output_size )
    {
        forget();
    }
}

////////////////
// resizeOnes //
////////////////
void GradNNetLayerModule::resizeOnes(int n)
{
    if (ones.length() < n) {
        ones.resize(n);
        ones.fill(1);
    } else if (ones.length() > n)
        ones.resize(n);
}



} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :

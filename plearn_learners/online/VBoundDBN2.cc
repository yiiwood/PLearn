// -*- C++ -*-

// VBoundDBN2.cc
//
// Copyright (C) 2007 yoshua Bengio
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

// Authors: yoshua Bengio

/*! \file VBoundDBN2.cc */


#include <plearn_learners/online/RBMMatrixConnection.h>
#include "VBoundDBN2.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    VBoundDBN2,
    "2-RBM DBN trained using Hinton's new variational bound of global likelihood",
    "The bound that is maximized is the following:\n"
    " log P(x) >= -FE1(x) + E_{P1(h|x)}[ FE1(h) - FE2(h) ] - log Z2\n"
    "where P1 and P2 are RBMs with Pi(x) = exp(-FEi(x))/Zi.\n"
);

//////////////////
// VBoundDBN2 //
//////////////////
VBoundDBN2::VBoundDBN2()
{
}

////////////////////
// declareOptions //
////////////////////
void VBoundDBN2::declareOptions(OptionList& ol)
{
    declareOption(ol, "rbm1", &VBoundDBN2::rbm1,
                  OptionBase::buildoption,
                  "First RBM, taking the DBN's input in its visible layer");
    declareOption(ol, "rbm2", &VBoundDBN2::rbm2,
                  OptionBase::buildoption,
                  "Second RBM, producing the DBN's output and generating internal representations.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void VBoundDBN2::build_()
{
    if (random_gen)
    {
        if (rbm1 && !rbm1->random_gen)
        {
            rbm1->random_gen = random_gen;
            rbm1->build();
            rbm1->forget();
        }
        if (rbm2 && !rbm2->random_gen)
        {
            rbm2->random_gen = random_gen;
            rbm2->build();
            rbm2->forget();
        }
    }
    if (ports.length()==0)
    {
        ports.append("input"); // 0
        ports.append("bound"); // 1
        ports.append("nll"); // 2
        ports.append("sampled_h"); // 3
        ports.append("global_improvement"); // 4
        ports.append("ph_given_v"); // 5
        ports.append("p2ph"); // 6
    }
}

///////////
// build //
///////////
void VBoundDBN2::build()
{
    inherited::build();
    build_();
}

////////////////////
// bpropAccUpdate //
////////////////////
void VBoundDBN2::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() && ports_gradient.length() == nPorts());
    PLASSERT( rbm1 && rbm2);

    Mat* input = ports_value[0];
    Mat* sampled_h_ = ports_value[3]; // a state if input is given
    Mat* global_improvement_ = ports_value[4]; // a state if input is given
    Mat* ph_given_v_ = ports_value[5]; // a state if input is given
    Mat* p2ph_ = ports_value[6]; // same story
    int mbs = input->length();
    PLASSERT( input && sampled_h_ && global_improvement_
              && ph_given_v_ && p2ph_);

    // do CD on rbm2
    rbm2->setAllLearningRates(rbm2->cd_learning_rate);
    rbm2->hidden_layer->setExpectations(*p2ph_);
    rbm2->hidden_layer->generateSamples();
    rbm2->sampleVisibleGivenHidden(rbm2->hidden_layer->samples);
    rbm2->computeHiddenActivations(rbm2->visible_layer->samples);
    rbm2->hidden_layer->computeExpectations();
    rbm2->visible_layer->update(*sampled_h_,rbm2->visible_layer->samples);
    rbm2->connection->update(*sampled_h_,*p2ph_,
                             rbm2->visible_layer->samples,
                             rbm2->hidden_layer->getExpectations());
    rbm2->hidden_layer->update(*p2ph_,rbm2->hidden_layer->getExpectations());

    // for now do the ugly hack, for binomial + MatrixConnection case
    PLASSERT(rbm1->visible_layer->classname()=="RBMBinomialLayer");
    PLASSERT(rbm1->hidden_layer->classname()=="RBMBinomialLayer");
    PLASSERT(rbm1->connection->classname() == "RBMMatrixConnection");
    Mat& weights = ((RBMMatrixConnection*)
                    get_pointer(rbm1->connection))->weights;
    static Mat delta_W;
    static Vec delta_hb;
    static Vec delta_vb1;
    static Vec delta_vb2;
    static Mat delta_h;
    delta_W.resize(rbm1->hidden_layer->size,rbm1->visible_layer->size);
    delta_hb.resize(rbm1->hidden_layer->size);
    delta_vb1.resize(rbm1->visible_layer->size);
    delta_vb2.resize(rbm1->visible_layer->size);
    delta_h.resize(mbs,rbm1->hidden_layer->size);

    // reconstruct the input
    rbm1->computeVisibleActivations(*sampled_h_);
    rbm1->visible_layer->computeExpectations();
    Mat reconstructed_v = rbm1->visible_layer->getExpectations();

    // compute RBM1 weight negative gradient
    //  dlogbound/dWij sampling approx = (ph_given_v[i] + (h[i]-ph_given_v[i])*global_improvement)*v[j] - h[i]*reconstructed_v[j]
    substract(*sampled_h_, *ph_given_v_, delta_h);
    multiply(delta_h, delta_h, global_improvement_->toVec());
    delta_h += *ph_given_v_;
    productScaleAcc(delta_W, delta_h, true, *input, false, 1., 0.);
    productScaleAcc(delta_W, *sampled_h_, true, reconstructed_v, false, -1., 1.);
    // update the weights
    multiplyAcc(weights, delta_W, rbm1->cd_learning_rate);

    // do the biases now
    //  dlogbound/dbi sampling approx = (ph_given_v[i] + (h[i]-ph_given_v[i])*global_improvement) - h[i]
    substract(delta_h, *sampled_h_, delta_h);
    columnSum(delta_h,delta_hb);
    multiplyAcc(rbm1->hidden_layer->bias,delta_hb,rbm1->cd_learning_rate);

    //  dlogbound/dji sampling approx = v[j] - reconstructed_v[j]
    columnSum(reconstructed_v,delta_vb1);
    columnSum(*input,delta_vb2);
    substract(delta_vb2,delta_vb1,delta_vb1);
    multiplyAcc(rbm1->visible_layer->bias,delta_vb1,rbm1->cd_learning_rate);

    // Ensure all required gradients have been computed.
    checkProp(ports_gradient);
}

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
// the default implementation returns false
bool VBoundDBN2::bpropDoesNothing()
{
}
*/

//////////////
// finalize //
//////////////
/* THIS METHOD IS OPTIONAL
void VBoundDBN2::finalize()
{
}
*/

////////////
// forget //
////////////
void VBoundDBN2::forget()
{
    if (rbm1 && rbm2)
    {
        rbm1->forget();
        rbm2->forget();
    }
}

///////////
// fprop //
///////////
void VBoundDBN2::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( rbm1 && rbm2);

    Mat* input = ports_value[0];
    Mat* bound = ports_value[1];
    Mat* nll = ports_value[2];
    Mat* sampled_h_ = ports_value[3]; // a state if input is given
    Mat* global_improvement_ = ports_value[4]; // a state if input is given
    Mat* ph_given_v_ = ports_value[5]; // a state if input is given
    Mat* p2ph_ = ports_value[6]; // same story

    // fprop has two modes:
    //  1) input is given (presumably for learning, or measuring bound or nll)
    //  2) input is not given and we want to generate one
    //

    // for learning or testing
    if (input && !input->isEmpty())
    {
        int mbs=input->length();
        FE1v.resize(mbs,1);
        FE1h.resize(mbs,1);
        FE2h.resize(mbs,1);
        Mat* sampled_h = sampled_h_?sampled_h_:&sampled_h_state;
        Mat* global_improvement = global_improvement_?global_improvement_:&global_improvement_state;
        Mat* ph_given_v = ph_given_v_?ph_given_v_:&ph_given_v_state;
        Mat* p2ph = p2ph_?p2ph_:&p2ph_state;
        sampled_h->resize(mbs,rbm1->hidden_layer->size);
        global_improvement->resize(mbs,1);
        ph_given_v->resize(mbs,rbm1->hidden_layer->size);

        // compute things needed for everything else

        rbm1->sampleHiddenGivenVisible(*input);
        *ph_given_v << rbm1->hidden_layer->getExpectations();
        *sampled_h << rbm1->hidden_layer->samples;
        rbm1->computeFreeEnergyOfVisible(*input,FE1v,false);
        rbm1->computeFreeEnergyOfHidden(*sampled_h,FE1h);
        rbm2->computeFreeEnergyOfVisible(*sampled_h,FE2h,false);
        p2ph->resize(mbs,rbm2->hidden_layer->size);
        *p2ph << rbm2->hidden_layer->getExpectations();
        substract(FE1h,FE2h,*global_improvement);

        if (bound) // actually minus the bound, to be in same units as nll, only computed exactly during test
        {
            PLASSERT(bound->isEmpty());
            bound->resize(mbs,1);

            if (rbm2->partition_function_is_stale && !during_training)
                rbm2->computePartitionFunction();
            *bound << FE1v;
            *bound -= *global_improvement;
            *bound += rbm2->log_partition_function;
        }
        if (nll) // exact -log P(input) = - log sum_h P2(h) P1(input|h)
        {
            PLASSERT( nll->isEmpty() );
            int n_h_configurations = 1 << rbm1->hidden_layer->size;
            if (all_h.length()!=n_h_configurations || all_h.width()!=rbm1->hidden_layer->size)
            {
                all_h.resize(n_h_configurations,rbm1->hidden_layer->size);
                for (int c=0;c<n_h_configurations;c++)
                {
                    int N=c;
                    for (int i=0;i<rbm1->hidden_layer->size;i++)
                    {
                        all_h(c,i) = N&1;
                        N >>= 1;
                    }
                }
            }
            // compute -log P2(h) for each possible h configuration
            if (rbm2->partition_function_is_stale && !during_training)
                rbm2->computePartitionFunction();
            neglogP2h.resize(n_h_configurations, 1);
            rbm2->computeFreeEnergyOfVisible(all_h, neglogP2h, false);
            neglogP2h += rbm2->log_partition_function;
            /*
            if (!during_training) {
                // Debug code to ensure probabilities sum to 1.
                real check = 0;
                real check2 = 0;
                for (int c = 0; c < n_h_configurations; c++) {
                    check2 += exp(- neglogP2h(c, 0));
                    if (c == 0)
                        check = - neglogP2h(c, 0);
                    else
                        check = logadd(check, - neglogP2h(c, 0));
                }
                pout << check << endl;
                pout << check2 << endl;
            }
            */
            rbm1->computeNegLogPVisibleGivenPHidden(*input,all_h,&neglogP2h,*nll);
        }
    }
    // Ensure all required ports have been computed.
    checkProp(ports_value);
}

//////////////////
// getPortIndex //
//////////////////
/* Optional
int VBoundDBN2::getPortIndex(const string& port)
{}
*/

//////////////
// getPorts //
//////////////
const TVec<string>& VBoundDBN2::getPorts() {
    return ports;
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& VBoundDBN2::getPortSizes() {
    PLASSERT(rbm1 && rbm2);
    if (sizes.width()!=2)
    {
        sizes.resize(nPorts(),2);
        sizes.fill(-1);
        sizes(0,1)=rbm1->visible_layer->size;
        sizes(1,1)=1;
        sizes(2,1)=1;
        sizes(3,1)=rbm1->hidden_layer->size;
        sizes(4,1)=1;
        sizes(5,1)=rbm1->hidden_layer->size;
        sizes(6,1)=rbm2->hidden_layer->size;
    }
    return sizes;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VBoundDBN2::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(rbm1, copies);
    deepCopyField(rbm2, copies);
    deepCopyField(FE1v, copies);
    deepCopyField(FE1h, copies);
    deepCopyField(FE2h, copies);
    deepCopyField(sampled_h_state, copies);
    deepCopyField(global_improvement_state, copies);
    deepCopyField(ph_given_v_state, copies);
    deepCopyField(p2ph_state, copies);
    deepCopyField(all_h, copies);
    deepCopyField(all_h, copies);
    deepCopyField(neglogP2h, copies);
    deepCopyField(ports, copies);
}

/////////////////////
// setLearningRate //
/////////////////////
/* OPTIONAL
// The default implementation raises a warning and does not do anything.
void VBoundDBN2::setLearningRate(real dynamic_learning_rate)
{
}
*/


}
// end of namespace PLearn


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

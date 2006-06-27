// -*- C++ -*-

// LinearInductiveTransferClassifier.cc
//
// Copyright (C) 2006 Hugo Larochelle 
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file LinearInductiveTransferClassifier.cc */


#include "LinearInductiveTransferClassifier.h"
#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/ArgmaxVariable.h>
#include <plearn/var/SourceVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/ExpVariable.h>
//#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include <plearn/var/OneHotVariable.h>
//#include <plearn/var/PowVariable.h>
#include <plearn/var/ProductTransposeVariable.h>
#include <plearn/var/ProductVariable.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/SumAbsVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/TransposeVariable.h>
//#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/VarRowsVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Var_utils.h>
#include <plearn/display/DisplayUtils.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/math/random.h>
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LinearInductiveTransferClassifier,
    "Linear classifier that uses class representations",
    "Linear classifier that uses class representations in\n"
    "order to make use of inductive transfer between classes.");

LinearInductiveTransferClassifier::LinearInductiveTransferClassifier() 
    : batch_size(1), 
      weight_decay(0), 
      penalty_type("L2_square"),
      initialization_method("uniform_linear"), 
      model_type("discriminative"),
      dont_consider_train_targets(false),
      use_bias_in_weights_prediction(false)
{
    random_gen = new PRandom();
}

void LinearInductiveTransferClassifier::declareOptions(OptionList& ol)
{
    declareOption(ol, "optimizer", &LinearInductiveTransferClassifier::optimizer, OptionBase::buildoption,
                  "Optimizer of the discriminative classifier");
    declareOption(ol, "batch_size", &LinearInductiveTransferClassifier::batch_size, OptionBase::buildoption, 
                  "How many samples to use to estimate the avergage gradient before updating the weights\n"
                  "0 is equivalent to specifying training_set->length() \n");
    declareOption(ol, "weight_decay", &LinearInductiveTransferClassifier::weight_decay, OptionBase::buildoption, 
                  "Global weight decay for all layers\n");
    declareOption(ol, "model_type", &LinearInductiveTransferClassifier::model_type, OptionBase::buildoption, 
                  "Model type. Choose between:\n"
                  " - \"discriminative\"             (multiclass classifier)\n"
                  " - \"discriminative_1_vs_all\"    (1 vs all multitask classier)\n"
                  " - \"generative\"                 (gaussian classifier)\n");
    declareOption(ol, "penalty_type", &LinearInductiveTransferClassifier::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L1_square\": square of the L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");
    declareOption(ol, "initialization_method", &LinearInductiveTransferClassifier::initialization_method, OptionBase::buildoption, 
                  "The method used to initialize the weights:\n"
                  " - \"normal_linear\"  = a normal law with variance 1/n_inputs\n"
                  " - \"normal_sqrt\"    = a normal law with variance 1/sqrt(n_inputs)\n"
                  " - \"uniform_linear\" = a uniform law in [-1/n_inputs, 1/n_inputs]\n"
                  " - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(n_inputs), 1/sqrt(n_inputs)]\n"
                  " - \"zero\"           = all weights are set to 0\n");    
    declareOption(ol, "paramsvalues", &LinearInductiveTransferClassifier::paramsvalues, OptionBase::learntoption, 
                  "The learned parameters\n");
    declareOption(ol, "class_reps", &LinearInductiveTransferClassifier::class_reps, OptionBase::buildoption, 
                  "Class vector representations\n");
    declareOption(ol, "dont_consider_train_targets", &LinearInductiveTransferClassifier::dont_consider_train_targets, OptionBase::buildoption, 
                  "Indication that the targets seen in the training set\n"
                  "should not be considered when tagging a new set\n");
    declareOption(ol, "use_bias_in_weights_prediction", &LinearInductiveTransferClassifier::use_bias_in_weights_prediction, OptionBase::buildoption, 
                  "Indication that a bias should be used for weights prediction\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LinearInductiveTransferClassifier::build_()
{
    /*
     * Create Topology Var Graph
     */

    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize and targetsize anyway...
    // Also, nothing is done if no layers need to be added
    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {
        if (seed_ != 0) random_gen->manual_seed(seed_);//random_gen->manual_seed(seed_);

        input = Var(inputsize(), "input");
        target = Var(1,"target");
        if(class_reps.size()<=0) 
            PLERROR("LinearInductiveTransferClassifier::build_(): class_reps is empty");
        noutputs = class_reps.length();
        buildTargetAndWeight();
        params.resize(0);

        Mat class_reps_to_use;
        if(use_bias_in_weights_prediction)
        {
            // Add column with 1s, to include bias
            Mat class_reps_with_bias(class_reps.length(), class_reps.width()+1);
            for(int i=0; i<class_reps_with_bias.length(); i++)
                for(int j=0; j<class_reps_with_bias.width(); j++)
                {
                    if(j==0)
                        class_reps_with_bias(i,j) = 1;
                    else
                        class_reps_with_bias(i,j) = class_reps(i,j-1);
                }
            class_reps_to_use = class_reps_with_bias;
        }
        else
        {
            class_reps_to_use = class_reps;
        }

        A = Var(inputsize_,class_reps_to_use.width());
        fillWeights(A,false);        
        s = Var(1,inputsize_,"sigma_square");
        s->value.fill(1);
        params.push_back(A);
        params.push_back(s);
        
        class_reps_var = new SourceVariable(class_reps_to_use);
        Var weights = productTranspose(A,class_reps_var);
        if(model_type == "discriminative" || model_type == "discriminative_1_vs_all")
            weights =vconcat(product(exp(s),square(weights)) & weights); // Making sure that this value is going to be positive
        else
            weights =vconcat(product(s,square(weights)) & weights);
        output = affine_transform(input, weights);

        Var sup_output;
        Var new_output;

        TVec<bool> class_tags(noutputs);
        Vec row(train_set.width());
        int target_class;
        class_tags.fill(0);
        for(int i=0; i<train_set.length(); i++)
        {
            train_set->getRow(i,row);
            target_class = (int) row[train_set->inputsize()];
            class_tags[target_class] = 1;
        }
        
        seen_targets.resize(0);
        unseen_targets.resize(0);
        for(int i=0; i<class_tags.length(); i++)
            if(class_tags[i])
                seen_targets.push_back(i);
            else
                unseen_targets.push_back(i);
        
        if(seen_targets.length() != class_tags.length())
        {
            sup_output = new VarRowsVariable(output,new SourceVariable(seen_targets));
            if(dont_consider_train_targets)
                new_output = new VarRowsVariable(output,new SourceVariable(unseen_targets));
            else
                new_output = output;
            Var sup_mapping = new SourceVariable(noutputs,1);
            Var new_mapping = new SourceVariable(noutputs,1);
            int sup_id = 0;
            int new_id = 0;
            for(int k=0; k<class_tags.length(); k++)
            {
                if(class_tags[k])
                {
                    sup_mapping->value[k] = sup_id;
                    new_mapping->value[k] = MISSING_VALUE;
                    sup_id++;
                }
                else
                {
                    sup_mapping->value[k] = MISSING_VALUE;
                    new_mapping->value[k] = new_id;
                    new_id++;
                }
            }
            sup_target = new VarRowsVariable(sup_mapping, target);
            if(dont_consider_train_targets)
                new_target = new VarRowsVariable(new_mapping, target);
            else
                new_target = target;
        }
        else
        {
            sup_output = output;
            new_output = output;
            sup_target = target;
            new_target = target;            
        }

        // Build costs
        if(model_type == "discriminative" || model_type == "discriminative_1_vs_all")
        {
            costs.resize(2);
            new_costs.resize(2);
            if(model_type == "discriminative")
            {
                sup_output = softmax(sup_output);
                costs[0] = neg_log_pi(sup_output,sup_target);
                costs[1] = classification_loss(sup_output, sup_target);
                new_output = softmax(new_output);
                new_costs[0] = neg_log_pi(new_output,new_target);
                new_costs[1] = classification_loss(new_output, new_target);
            }
            if(model_type == "discriminative_1_vs_all")
            {
                
                costs[0] = stable_cross_entropy(sup_output, onehot(seen_targets.length(),sup_target));
                costs[1] = classification_loss(sigmoid(sup_output), sup_target);
                if(dont_consider_train_targets)
                    new_costs[0] = stable_cross_entropy(new_output, onehot(unseen_targets.length(),new_target));
                else
                    new_costs[0] = stable_cross_entropy(new_output, onehot(noutputs,new_target));
                new_costs[1] = classification_loss(sigmoid(new_output), new_target);
            }
        }
        else if(model_type == "generative")
        {
            costs.resize(1);
            costs[0] = classification_loss(sup_output, sup_target);
            new_costs.resize(1);
            new_costs[0] = classification_loss(new_output, new_target);
        }
        else PLERROR("LinearInductiveTransferClassifier::build_(): model_type \"%s\" invalid",model_type.c_str());


        string pt = lowerstring( penalty_type );
        if( pt == "l1" )
            penalty_type = "L1";
        else if( pt == "l1_square" || pt == "l1 square" || pt == "l1square" )
            penalty_type = "L1_square";
        else if( pt == "l2_square" || pt == "l2 square" || pt == "l2square" )
            penalty_type = "L2_square";
        else if( pt == "l2" )
        {
            PLWARNING("L2 penalty not supported, assuming you want L2 square");
            penalty_type = "L2_square";
        }
        else
            PLERROR("penalty_type \"%s\" not supported", penalty_type.c_str());

        buildPenalties();
        Var train_costs = hconcat(costs);
        test_costs = hconcat(new_costs);

        // Apply penalty to cost.
        // If there is no penalty, we still add costs[0] as the first cost, in
        // order to keep the same number of costs as if there was a penalty.
        if(penalties.size() != 0) {
            if (weightsize_>0)
                training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties))
                                        & (train_costs*sampleweight));
            else 
                training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & train_costs);
        }
        else {
            if(weightsize_>0) {
                training_cost = hconcat(costs[0]*sampleweight & train_costs*sampleweight);
            } else {
                training_cost = hconcat(costs[0] & train_costs);
            }
        }

        training_cost->setName("training_cost");
        test_costs->setName("test_costs");


        if((bool)paramsvalues && (paramsvalues.size() == params.nelems()))
            params << paramsvalues;
        else
            paramsvalues.resize(params.nelems());
        params.makeSharedValue(paramsvalues);
        
        // Build functions.
        buildFuncs(input, output, target, sampleweight);
        
        // Reinitialize the optimization phase
        if(optimizer)
            optimizer->reset();
        stage = 0;        
    }
}


// ### Nothing to add here, simply calls build_
void LinearInductiveTransferClassifier::build()
{
    inherited::build();
    build_();
}


void LinearInductiveTransferClassifier::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(class_reps, copies);
    deepCopyField(optimizer, copies);
    deepCopyField(params, copies);
    deepCopyField(paramsvalues, copies);
    deepCopyField(invars, copies);
    deepCopyField(costs, copies);
    deepCopyField(new_costs, copies);
    deepCopyField(penalties, copies);
    deepCopyField(seen_targets, copies);
    deepCopyField(unseen_targets, copies);

    deepCopyField(f, copies);
    deepCopyField(test_costf, copies);
    deepCopyField(output_and_target_to_cost, copies);

    varDeepCopyField(A, copies);
    varDeepCopyField(s, copies);
    varDeepCopyField(class_reps_var, copies);

    varDeepCopyField(input, copies);
    varDeepCopyField(output, copies);
    varDeepCopyField(sup_output, copies);
    varDeepCopyField(new_output, copies);
    varDeepCopyField(target, copies);
    varDeepCopyField(sup_target, copies);
    varDeepCopyField(new_target, copies);
    varDeepCopyField(sampleweight, copies);
    varDeepCopyField(training_cost, copies);
    varDeepCopyField(test_costs, copies);

    //PLERROR("LinearInductiveTransferClassifier::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int LinearInductiveTransferClassifier::outputsize() const
{
    if(output)
        return output->size();
    else
        return 0;
}

void LinearInductiveTransferClassifier::forget()
{
    if(optimizer)
        optimizer->reset();
    stage = 0;
    
    fillWeights(A,false);
    s->value.fill(1);
 
    build();
}
    
void LinearInductiveTransferClassifier::train()
{
    if(!train_set)
        PLERROR("In DeepFeatureExtractor::train, you did not setTrainingSet");
    
    if(!train_stats)
        PLERROR("In DeepFeatureExtractor::train, you did not setTrainStatsCollector");

    int l = train_set->length();  

    if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
        build();
    
    if(model_type == "discriminative" || model_type == "discriminative_1_vs_all")
    {
        // number of samples seen by optimizer before each optimizer update
        int nsamples = batch_size>0 ? batch_size : l;
        Func paramf = Func(invars, training_cost); // parameterized function to optimize
        Var totalcost = meanOf(train_set, paramf, nsamples);
        if(optimizer)
        {
            optimizer->setToOptimize(params, totalcost);  
            optimizer->build();
        }
        else PLERROR("DeepFeatureExtractor::train can't train without setting an optimizer first!");

        // number of optimizer stages corresponding to one learner stage (one epoch)
        int optstage_per_lstage = l/nsamples;

        ProgressBar* pb = 0;
        if(report_progress)
            pb = new ProgressBar("Training " + classname() + " from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

        int initial_stage = stage;
        bool early_stop=false;
        //displayFunction(paramf, true, false, 250);
        while(stage<nstages && !early_stop)
        {
            optimizer->nstages = optstage_per_lstage;
            train_stats->forget();
            optimizer->early_stop = false;
            optimizer->optimizeN(*train_stats);
            // optimizer->verifyGradient(1e-4); // Uncomment if you want to check your new Var.
            train_stats->finalize();
            if(verbosity>2)
                cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
            ++stage;
            if(pb)
                pb->update(stage-initial_stage);
        }
        if(verbosity>1)
            cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

        if(pb)
            delete pb;
    }
    else
    {
        Mat ww(class_reps_var->width(),class_reps_var->width()); ww.fill(0);
        Mat ww_inv(class_reps_var->width(),class_reps_var->width());
        Mat xw(inputsize(),class_reps_var->width()); xw.fill(0);
        Vec input, target;
        real weight;
        input.resize(train_set->inputsize());
        target.resize(train_set->targetsize());        
        for(int i=0; i<train_set->length(); i++)
        {
            train_set->getExample(i,input,target,weight);
            if(weightsize()>0)
            {
                externalProductScaleAcc(ww,class_reps_var->matValue((int)target[0]),class_reps_var->matValue((int)target[0]),weight);
                externalProductScaleAcc(xw,input,class_reps_var->matValue((int)target[0]),weight);
            }
            else
            {
                externalProductAcc(ww,class_reps_var->matValue((int)target[0]),class_reps_var->matValue((int)target[0]));
                externalProductAcc(xw,input,class_reps_var->matValue((int)target[0]));
            }
        }
        matInvert(ww,ww_inv);
        A->value.fill(0);
        productAcc(A->matValue, xw, ww_inv);
        
        s->value.fill(0);
        Vec sample(s->size());
        Vec weights(inputsize());
        real sum = 0;
        for(int i=0; i<train_set->length(); i++)
        {
            train_set->getExample(i,input,target,weight);
            product(weights,A->matValue,class_reps_var->matValue((int)target[0]));
            if(weightsize()>0)
            {
                diffSquareMultiplyAcc(s->value,weights,input,weight);
                sum += weight;
            }
            else
            {
                diffSquareMultiplyAcc(s->value,weights,input,1.0);
                sum++;
            }
        }
        s->value /= sum;
        s->value *= weight_decay+1;

        if(verbosity > 2)
        {
            Func paramf = Func(invars, training_cost);
            paramf->recomputeParents();
            real mean_cost = 0;
            Vec cost(2);
            Vec row(train_set->width());
            for(int i=0; i<train_set->length(); i++)
            {
                train_set->getRow(i,row);
                paramf->fprop(row.subVec(0,inputsize()+targetsize()),cost);
                mean_cost += cost[1];
            }
            mean_cost /= train_set->length();
            cout << "Train class error: " << mean_cost << endl;
        }
    }
    // Hugo: I don't know why we have to do this?!?
    output_and_target_to_cost->recomputeParents();
    test_costf->recomputeParents();
}

void LinearInductiveTransferClassifier::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());
    f->fprop(input,output);
}    

void LinearInductiveTransferClassifier::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
    if(seen_targets.find(target[0])>=0)
        sup_output_and_target_to_cost->fprop(output&target, costs);
    else
        output_and_target_to_cost->fprop(output&target, costs);
}

void LinearInductiveTransferClassifier::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
    outputv.resize(outputsize());
    if(seen_targets.find(targetv[0])>=0)
        sup_test_costf->fprop(inputv&targetv, outputv&costsv);
    else
        test_costf->fprop(inputv&targetv, outputv&costsv);
}

TVec<string> LinearInductiveTransferClassifier::getTestCostNames() const
{
    TVec<string> costs_str;
    if(model_type == "discriminative" || model_type == "discriminative_1_vs_all")
    {
        costs_str.resize(2);
        if(model_type == "discriminative")
        {
            costs_str[0] = "NLL";
            costs_str[1] = "class_error";
        }
        if(model_type == "discriminative_1_vs_all")
        {
            costs_str[0] = "cross_entropy";
            costs_str[1] = "class_error";
        }
    }
    else if(model_type == "generative")
    {
        costs_str.resize(1);
        costs_str[0] = "class_error";
    }
    return costs_str;
}

TVec<string> LinearInductiveTransferClassifier::getTrainCostNames() const
{
    return getTestCostNames();
}

void LinearInductiveTransferClassifier::buildTargetAndWeight() {
    //if(nhidden_schedule_current_position >= nhidden_schedule.length())
    if(targetsize() > 0)
    {
        target = Var(targetsize(), "target");
        if(weightsize_>0)
        {
            if (weightsize_!=1)
                PLERROR("In NNet::buildTargetAndWeight - Expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
            sampleweight = Var(1, "weight");
        }
    }
}

void LinearInductiveTransferClassifier::buildPenalties() {
    penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
    if(weight_decay > 0)
    {
        penalties.append(affine_transform_weight_penalty(A, weight_decay, (use_bias_in_weights_prediction ? 0 : weight_decay), penalty_type));
    }
}

void LinearInductiveTransferClassifier::fillWeights(const Var& weights, bool fill_first_row, real fill_with_this) {
    if (initialization_method == "zero") {
        weights->value->clear();
        return;
    }
    real delta;
    int is = weights.length();
    if (fill_first_row)
        is--; // -1 to get the same result as before.
    if (initialization_method.find("linear") != string::npos)
        delta = 1.0 / real(is);
    else
        delta = 1.0 / sqrt(real(is));
    if (initialization_method.find("normal") != string::npos)
        random_gen->fill_random_normal(weights->value, 0, delta);
    else
        random_gen->fill_random_uniform(weights->value, -delta, delta);
    if (fill_first_row)
        weights->matValue(0).fill(fill_with_this);
}

void LinearInductiveTransferClassifier::buildFuncs(const Var& the_input, const Var& the_output, const Var& the_target, const Var& the_sampleweight){
    invars.resize(0);
    VarArray outvars;
    VarArray testinvars;
    if (the_input)
    {
        invars.push_back(the_input);
        testinvars.push_back(the_input);
    }
    if (the_output)
        outvars.push_back(the_output);
    if(the_target)
    {
        invars.push_back(the_target);
        testinvars.push_back(the_target);
        outvars.push_back(the_target);
    }
    if(the_sampleweight)
    {
        invars.push_back(the_sampleweight);
    }
    f = Func(the_input, the_output);
    test_costf = Func(testinvars, the_output&test_costs);
    test_costf->recomputeParents();
    output_and_target_to_cost = Func(outvars, test_costs); 
    output_and_target_to_cost->recomputeParents();

    VarArray sup_outvars;
    sup_test_costf = Func(testinvars, the_output&hconcat(costs));
    sup_test_costf->recomputeParents();
    sup_output_and_target_to_cost = Func(outvars, hconcat(costs)); 
    sup_output_and_target_to_cost->recomputeParents();
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
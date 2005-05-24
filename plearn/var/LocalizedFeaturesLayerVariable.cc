// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 2005 Yoshua Bengio

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
   * $Id: LocalizedFeaturesLayerVariable.cc,v 1.4 2005/05/24 15:42:12 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "LocalizedFeaturesLayerVariable.h"
#include <plearn/math/random.h>
#include <plearn/math/BottomNI.h>
#include <plearn/math/TMat_maths_impl.h>
#include <plearn/math/TMat_maths_specialisation.h>

namespace PLearn {
using namespace std;


/** LocalizedFeaturesLayerVariable **/

// Single layer of a neural network.

PLEARN_IMPLEMENT_OBJECT(LocalizedFeaturesLayerVariable,
                        "Single layer of a neural network, with local connectivity on a set of localized features.\n",
                        "Each feature is associated with a location in some low-dimensional space\n"
                        "and each hidden unit takes input only from a subset of features that are\n"
                        "in some local region in that space.\n"
                        "The user can specify the feature subsets or specify the low-dimensional embedding\n"
                        "of the features. In the latter case the subsets are derived according to one of\n"
                        "several algorithms. The default one (knn_subsets) is simply that there is one\n"
                        "subset per feature, with n_neighbors_per_subset+1 features per subset, that include\n"
                        "the 'central' feature and its n_neighbors_per_subset embedding neighbors.\n"
                        "\n"
                        "TODO IN A FUTURE RELEASE(?):\n"
                        "The learning algorithm that estimates feature locations or directly the graph\n"
                        "that defines subsets of features could be embedded in this class.\n"
                        );

LocalizedFeaturesLayerVariable::LocalizedFeaturesLayerVariable()
  : n_features(-1),
    backprop_to_inputs(false),
    n_hidden_per_subset(1),
    knn_subsets(true),
    n_neighbors_per_subset(-1),
    gridding_subsets(false),
    center_on_feature_locations(true)
{
}

void
LocalizedFeaturesLayerVariable::build()
{
    inherited::build();
    build_();
}

void
LocalizedFeaturesLayerVariable::build_()
{
  if (varray.length() == 0)
    // Cannot do anything yet.
    return;
  computeSubsets();
  if (   varray.size() != 3
         || n_connections != varray[1]->value.size()
         || n_hidden_per_subset*n_subsets != varray[2]->value.size()  )
  {
    varray.resize(3);
    varray[1] = Var(n_connections);
    varray[2] = Var(n_hidden_per_subset*n_subsets);
    // varray[0] = input
    // varray[1] = connection weights
    // varray[2] = biases
  }
  if (varray[0]) {
    if (n_features != varray[0]->value.size())
      PLERROR("In LocalizedFeaturesLayerVariable: input var 0 (features) should have size = %d = n_features, but is %d\n",
              n_features, varray[0]->value.size());
    /* Cannot happen.
    if (n_connections != varray[1]->value.size())
      PLERROR("In LocalizedFeaturesLayerVariable: input var 1 (weights) should have size = %d = n_connections, but is %d\n",
              n_connections, varray[1]->value.size());
    if (n_hidden_per_subset*n_subsets != varray[2]->value.size())
      PLERROR("In LocalizedFeaturesLayerVariable: input var 2 (biases) should have size = %d = n_hidden, but is %d\n",
              n_hidden_per_subset*n_subsets,varray[2]->value.size());
    */

    // Initialize parameters.
    real n_inputs_per_neuron = n_connections / real(n_hidden_per_subset*n_subsets);
    real delta = 1/n_inputs_per_neuron;
    fill_random_uniform(varray[1]->value, -delta, delta);
    varray[2]->value.clear();
  }
}

void LocalizedFeaturesLayerVariable::computeSubsets()
{
  if (knn_subsets)
  {
    n_features = varray[0]->value.size();    // Get n_features from first var.
    n_subsets = n_features;
    n_connections = (1+n_neighbors_per_subset) * n_subsets * n_hidden_per_subset;
    feature_subsets.resize(n_subsets);
    BottomNI<real> lowest_distances;
    Vec center(feature_locations->width());
    Vec feature(feature_locations->width());
    for (int s=0;s<n_subsets;s++)
    {
      feature_subsets[s].resize(n_neighbors_per_subset + 1);
      // find k-nearest neighbors of feature s according to feature_locations
      lowest_distances.init(n_neighbors_per_subset);
      feature_locations->getRow(s, center);
      for (int f=0;f<n_features;f++)
        if (f!=s)
        {
          feature_locations->getRow(f, feature);
          real dist = powdistance(center, feature);
          lowest_distances.update(dist,f);
        }
      TVec< pair<real,int> > neighbors = lowest_distances.getBottomN();
      feature_subsets[s][0] = s;
      for (int k=0;k<n_neighbors_per_subset;k++)
        feature_subsets[s][k+1] = neighbors[k].second;
    }
  }
  else
    PLERROR("In LocalizedFeaturesLayerVariable: currently the only method for computing subsets is 'knn_subsets'.\n");
}

////////////////////
// declareOptions //
////////////////////
void LocalizedFeaturesLayerVariable::declareOptions(OptionList& ol)
{
  declareOption(ol, "backprop_to_inputs", &LocalizedFeaturesLayerVariable::backprop_to_inputs, OptionBase::buildoption, 
                "    If true then gradient is propagated to the inputs. When this object is the first layer\n"
                "    of a neural network, it is more efficient to set this option to false (which is its default).\n");

  declareOption(ol, "feature_locations", &LocalizedFeaturesLayerVariable::feature_locations, 
                OptionBase::buildoption, 
                "    (n_features x n_dim) matrix assigning each feature to a location in n_dimensional space.\n"
                "    If feature_subsets is directly provided, it is not necessary to provide this option, as\n"
                "    the feature locations are only used to infer the feature_subsets.\n");

  declareOption(ol, "feature_subsets", &LocalizedFeaturesLayerVariable::feature_subsets, 
                OptionBase::buildoption, 
                "    Sequence of vector indices, one sequence per subset, containing indices of features\n"
                "    associated with each subset. If not specified (empty sequence) it will be inferred\n"
                "    using the feature_locations (which MUST be specified, in that case).\n");

  declareOption(ol, "n_hidden_per_subset", &LocalizedFeaturesLayerVariable::n_hidden_per_subset, 
                OptionBase::buildoption, 
                "    Number of hidden units per unique feature subset.\n");

  declareOption(ol, "knn_subsets", &LocalizedFeaturesLayerVariable::knn_subsets, 
                OptionBase::buildoption, 
                "    Whether to infer feature_subsets using the k-nearest-neighbor algorithm or not.\n"
                "    This option is only used when feature_subsets is not directly provided by the user.\n"
                "    Each subset will have size 'n_neighbors_per_subset' and there will be one subset\n"
                "    per feature, comprising that feature and its 'n_neighbors_per_subset' neighbors\n"
                "    in terms of Euclidean distance in feature locations.\n");

  declareOption(ol, "n_neighbors_per_subset", &LocalizedFeaturesLayerVariable::n_neighbors_per_subset, 
                OptionBase::buildoption, 
                "   Number of feature neighbors to consider in each subset, if knn_subsets == true and\n"
                "   feature_subsets is not specified directly by the user.\n");

  declareOption(ol, "gridding_subsets", &LocalizedFeaturesLayerVariable::gridding_subsets, 
                OptionBase::buildoption, 
                "    Whether to infer feature_subsets using a gridding algorithm.\n"
                "    NOT IMPLEMENTED YET.\n");

  declareOption(ol, "center_on_feature_locations", &LocalizedFeaturesLayerVariable::center_on_feature_locations,
                OptionBase::buildoption, 
                "    If gridding_subsets == true, whether to center each subset window on feature locations\n"
                "    or on regularly spaced centers in a volume.\n"
                "    NOT IMPLEMENTED YET.\n");


  inherited::declareOptions(ol);
}


void LocalizedFeaturesLayerVariable::recomputeSize(int& l, int& w) const
{
  if (varray.length() >= 2 && varray[0] && varray[1]) {
    l = n_hidden_per_subset * n_subsets;
    w = 1;
    // TODO Seems different from the FNetVariable
    // TODO A resize in build ?
  } else
    l = w = 0;
}

void LocalizedFeaturesLayerVariable::fprop()
{
  real* x = varray[0]->valuedata;
  real* y = valuedata;
  real* w = varray[1]->valuedata;
  real* b = varray[2]->valuedata;
  for (int s=0;s<n_subsets;s++)
  {
    TVec<int> subset = feature_subsets[s];
    int subset_size = subset.length();
    for (int k=0;k<n_hidden_per_subset;k++,b++,y++)
    {
      real act = *b;
      for (int j=0;j<subset_size;j++,w++)
        act += *w * x[subset[j]];
      *y = tanh(act);
    }
  }
}


void LocalizedFeaturesLayerVariable::bprop()
{
  real* x = varray[0]->valuedata;
  real* dx = varray[0]->gradientdata;
  real* y = valuedata;
  real* dy = gradientdata;
  real* w = varray[1]->valuedata;
  real* dw = varray[1]->gradientdata;
  real* db = varray[2]->gradientdata;
  for (int s=0;s<n_subsets;s++)
  {
    TVec<int> subset = feature_subsets[s];
    int subset_size = subset.length();
    for (int k=0;k<n_hidden_per_subset;k++,db++,y++,dy++)
    {
      // TODO Check this derivative.
      real dact = *dy * *y * (1 - *y);
      *db += dact;
      for (int j=0;j<subset_size;j++,dw++)
        *dw += dact * x[subset[j]];
      if (backprop_to_inputs)
        for (int j=0;j<subset_size;j++,w++)
          dx[subset[j]] += dact * *w;
    }
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LocalizedFeaturesLayerVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(feature_locations, copies);
  deepCopyField(feature_subsets, copies);
}

} // end of namespace PLearn



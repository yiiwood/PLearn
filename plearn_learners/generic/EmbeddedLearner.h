// -*- C++ -*-

// EmbeddedLearner.h
// 
// Copyright (C) 2002 Frederic Morin
// Copyright (C) 2003 Pascal Vincent
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
   * $Id: EmbeddedLearner.h,v 1.7 2003/08/13 08:13:46 plearner Exp $ 
   ******************************************************* */

/*! \file EmbeddedLearner.h */
#ifndef EmbeddedLearner_INC
#define EmbeddedLearner_INC

#include "PLearner.h"

namespace PLearn <%
using namespace std;

// ###### EmbeddedLearner ######################################################

class EmbeddedLearner: public PLearner
{
public:

  // build option
    PP<PLearner> learner_;


    typedef PLearner inherited;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    EmbeddedLearner();

  // ******************
  // * Object methods *
  // ******************

private: 
    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();

protected: 
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);
public:
    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Provides a help message describing this class
    static string help();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(EmbeddedLearner);

    // *******************
    // * PLearner methods *
    // *******************
    
  virtual int inputsize() const;
  
  virtual int targetsize() const; 
  
  virtual int outputsize() const;

  virtual void forget();

    virtual void train();

    virtual void computeOutput(const Vec& input, Vec& output) const;

    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;
                                
    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;

    virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
      
    virtual TVec<string> getTestCostNames() const;

    virtual TVec<string> getTrainCostNames() const;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(EmbeddedLearner);
  
%> // end of namespace PLearn

#endif

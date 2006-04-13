// -*- C++ -*-

// RBMLayer.h
//
// Copyright (C) 2006 Dan Popovici
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

// Authors: Dan Popovici

/*! \file RBMLayer.h */


#ifndef RBMLayer_INC
#define RBMLayer_INC

#include <plearn/base/Object.h>

namespace PLearn {
using namespace std;

// forward declarations
class RBMParameters;
class PRandom;

/**
 * Virtual class for a layer in an RBM.
 *
 * @todo: yes
 */
class RBMLayer: public Object
{
    typedef Object inherited;

public:
    //#####  Public Build Options  ############################################

    //! Number of units
    int size;

    //! Each character of this string describes the type of an up unit:
    //!   - 'l' if the energy function of this unit is linear (binomial or
    //!     multinomial unit),
    //!   - 'q' if it is quadratic (for a gaussian unit)
    string units_types;

    //! Random number generator
    PP<PRandom> random_gen;

    //#####  Not Options  #####################################################

    //! values allowing to know the distribution :
    //! the activation value (before sigmoid) for a binomial input,
    //! the couple (mu, sigma) for a gaussian unit.
    Vec activations;

    //! Contains a sample of the random variable in this layer:
    Vec sample;

    //! Contains the expected value of the random variable in this layer:
    Vec expectation;

    //! flags that expectation was computed based on most recently computed value of activation
    bool expectation_is_up_to_date;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMLayer();

    // Your other public member functions go here

    //! Uses "rbmp" to obtain the activations of unit "i" of this layer.
    //! This activation vector is computed by the "i+offset"-th unit of "rbmp"
    virtual void getUnitActivations( int i, PP<RBMParameters> rbmp,
                                     int offset=0 ) = 0;

    //! Uses "rbmp" to obtain the activations of all units in this layer.
    //! Unit 0 of this layer corresponds to unit "offset" of "rbmp".
    virtual void getAllActivations( PP<RBMParameters> rbmp,
                                    int offset=0 ) = 0;

    //! generate a sample, and update the sample field
    virtual void generateSample() = 0 ;

    //! compute the expectation
    virtual void computeExpectation() = 0 ;

    //! return units_types
    inline string getUnitsTypes()
    {
        return units_types;
    }


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(RBMLayer);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################


protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMLayer);

} // end of namespace PLearn

#endif


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

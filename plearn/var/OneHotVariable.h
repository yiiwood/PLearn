// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: OneHotVariable.h,v 1.5 2004/04/27 16:03:35 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef OneHotVariable_INC
#define OneHotVariable_INC

#include "UnaryVariable.h"

namespace PLearn {
using namespace std;



//!  Represents a vector of a given lenth, that has value 1 at the index 
//!  given by another variable and 0 everywhere else 
class OneHotVariable: public UnaryVariable
{
  typedef UnaryVariable inherited;

protected:
  real hotvalue;
  real coldvalue;
  int length_;

public:
  //!  Default constructor for persistence
  OneHotVariable() : hotvalue(), coldvalue() {}
  OneHotVariable(int thelength, Variable* index, real the_coldvalue, real the_hotvalue);

  PLEARN_DECLARE_OBJECT(OneHotVariable);
  static void declareOptions(OptionList &ol);

  virtual void build();

  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();

protected:
    void build_();
};

DECLARE_OBJECT_PTR(OneHotVariable);

inline Var onehot(int the_length, Var hotindex, real coldvalue=0.0, real hotvalue=1.0)
{ return new OneHotVariable(the_length, hotindex, coldvalue, hotvalue); }


} // end of namespace PLearn

#endif 

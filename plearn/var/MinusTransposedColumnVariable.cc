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
   * $Id: MinusTransposedColumnVariable.cc,v 1.3 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MinusTransposedColumnVariable.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** MinusTransposedColumnVariable **/

MinusTransposedColumnVariable::MinusTransposedColumnVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isColumnVec())
    PLERROR("IN MinusTransposedColumnVariable: input2 is not a column");
  if(input2->length() != input1->width())
    PLERROR("IN MinusTransposedColumnVariable: the width() of input1 and the length() of input2 differ");
}


PLEARN_IMPLEMENT_OBJECT(MinusTransposedColumnVariable, "ONE LINE DESCR", "NO HELP");


void MinusTransposedColumnVariable::recomputeSize(int& l, int& w) const
{ l=input1->length(); w=input1->width(); }








void MinusTransposedColumnVariable::fprop()
{
  int k=0;
  real* i2 = input2->valuedata;
  real* i1 = input1->valuedata;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = i1[k] - i2[j];
}


void MinusTransposedColumnVariable::bprop()
{
  int k=0;
  real* i2g = input2->gradientdata;
  real* i1g = input1->gradientdata;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        i1g[k] += gradientdata[k];
        i2g[j] -= gradientdata[k];
      }
}


void MinusTransposedColumnVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(-columnSum(g));
}



%> // end of namespace PLearn



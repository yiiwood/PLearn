// -*- C++ -*-

// diff.h
//
// Copyright (C) 2005 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file diff.h */


#ifndef diff_INC
#define diff_INC

#include <vector>
#include <plearn/base/PP.h>
#include <plearn/base/Option.h>
#include <plearn/base/OptionBase.h>
#include <plearn/base/tostring.h>
#include <plearn/io/openString.h>
#include <plearn/math/TVec_decl.h>
//#include <plearn/vmat/VMatrix.h>

namespace PLearn {

//! Forward declarations.
class Object;
template<class ObjectType, class OptionType> class Option;
//template <class T> class TVec;
class VMat;
class VMatrix;
 
//! Default diff function: compare the two strings.
int diff(const string& refer, const string& other, const OptionBase* opt, vector<string>& diffs);

//! Specialization for TVec<>.
template<class ObjectType, class VecElementType>
int diff(const string& refer, const string& other, const Option<ObjectType, TVec<VecElementType> >* opt, vector<string>& diffs)
{
  pout << "Calling diff(..., const Option<ObjectType, TVec<T> > opt, ...)" << endl;
  int n_diffs = 0;
  TVec<VecElementType> refer_vec;
  TVec<VecElementType> other_vec;
  string option = opt->optionname();
  PStream in;
  in = openString(refer, PStream::plearn_ascii);
  in >> refer_vec;
  in = openString(other, PStream::plearn_ascii);
  in >> other_vec;
  int n = refer_vec.length();
  if (other_vec.length() != n)
    // If the two vectors do not have the same size, no need to go further.
    n_diffs += diff(tostring(n), tostring(other_vec.length()),
                    opt->optionname() + ".length", diffs);
  else {
    PP<OptionBase> option_elem = new Option<ObjectType, VecElementType>
      ("", 0, 0, TypeTraits<VecElementType>::name(), "", "");
    string refer_i, other_i;
    for (int i = 0; i < n; i++) {
      option_elem->setOptionName(opt->optionname() + "[" + tostring(i) + "]");
      PStream out = openString(refer_i, PStream::plearn_ascii, "w");
      out << refer_vec[i];
      out.flush();
      out = openString(other_i, PStream::plearn_ascii, "w");
      out << other_vec[i];
      out.flush();
      n_diffs += option_elem->diff(refer_i, other_i, diffs);
    }
  }
  return n_diffs;
}

//! Specialization for PP<>.
template<class ObjectType, class PointedType>
int diff(const string& refer, const string& other, const Option<ObjectType, PP<PointedType> >* opt, vector<string>& diffs)
{
  pout << "Calling diff with Option< ObjectType, " << opt->optiontype() << " >" << endl;
  PP<PointedType> refer_obj;
  PP<PointedType> other_obj;
  PStream in = openString(refer, PStream::plearn_ascii);
  in >> refer_obj;
  in = openString(other, PStream::plearn_ascii);
  in >> other_obj;
  int n_diffs = diff(static_cast<PointedType*>(refer_obj),
                     static_cast<PointedType*>(other_obj), diffs);
  addDiffPrefix(opt->optionname() + ".", diffs, n_diffs);
  return n_diffs;
}

//! Specialization for VMat.
template<class ObjectType>
int diff(const string& refer, const string& other, const Option<ObjectType, VMat >* opt, vector<string>& diffs)
{
  return diff(refer, other,
              (Option<ObjectType, PP<VMatrix> >*) opt, diffs);
}

//! Add 'prefix' in front of the last 'n' difference names in 'diffs'.
void addDiffPrefix(const string& prefix, vector<string>& diffs, int n);

/*
template<class ObjectType, class VecElementType>
int diff(PP<Object> refer, PP<Object> other, const Option<ObjectType, TVec<VecElementType> >* opt, vector<string>& diffs)
{
  string refer_str, other_str;
}
*/

int diff(PP<Object> refer, PP<Object> other, vector<string>& diffs);

//! If 'refer != other, add a new difference with name 'name', reference
//! value 'refer' and other value 'other' to given vector of differences.
//! 'is_diff' is increased by 1 in this case (otherwise it is not changed).
// TODO Update comment.
int diff(const string& refer, const string& other, const string& name,
         vector<string>& diffs);

} // end of namespace PLearn

#endif


// -*- C++ -*-

// FilePStreamBuf.h
//
// Copyright (C) 2004 Pascal Vincent 
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
   * $Id: FilePStreamBuf.h,v 1.4 2005/01/07 23:51:22 chrish42 Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file FilePStreamBuf.h */


#ifndef FilePStreamBuf_INC
#define FilePStreamBuf_INC

#include "PStreamBuf.h"

namespace PLearn {
using namespace std;

class FilePStreamBuf: public PStreamBuf
{

private:
  
  typedef PStreamBuf inherited;

protected:
  // *********************
  // * protected options *
  // *********************
  FILE* f;

public:

  // ************************
  // * public build options *
  // ************************
  string url;    
  string openmode;

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  FilePStreamBuf();

  virtual ~FilePStreamBuf();

  static string getFilePathFromURL(string fileurl);

  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (PLEASE IMPLEMENT IN .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (PLEASE IMPLEMENT IN .cc)
  static void declareOptions(OptionList& ol);

  virtual streamsize read_(char* p, streamsize n);

  //! writes exactly n characters from p (unbuffered, must flush)
  virtual void write_(const char* p, streamsize n);


public:
  // Declares other standard object methods
  //  If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT(FilePStreamBuf);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  // (PLEASE IMPLEMENT IN .cc)
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  virtual bool eof() const;
};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(FilePStreamBuf);
  
} // end of namespace PLearn

#endif

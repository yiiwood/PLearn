// -*- C++ -*-

// RPPath.cc
//
// Copyright (C) 2008 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file RPPath.cc */


#include "RPPath.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RPPath,
    "Remote interface for a PPath",
    "This class is intended to manipulate file and directory paths from\n"
    "outside PLearn.\n"
    "Currently it can only be used to convert a PPath to its absolute or\n"
    "canonical representation, but more forwarded methods may be added in\n"
    "the future."

);

////////////
// RPPath //
////////////
RPPath::RPPath()
{}

////////////////////
// declareOptions //
////////////////////
void RPPath::declareOptions(OptionList& ol)
{
    declareOption(ol, "path", &RPPath::path,
            OptionBase::buildoption,
        "Path (to file or directory) that is being manipulated.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////////////
// declareMethods //
////////////////////
void RPPath::declareMethods(RemoteMethodMap& rmm)
{
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(rmm, "absolute", &RPPath::absolute,
            (BodyDoc("Return the absolute path as a RPPath object.")));

    declareMethod(rmm, "canonical", &RPPath::canonical,
            (BodyDoc("Return the canonic path.")));
}

///////////
// build //
///////////
void RPPath::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void RPPath::build_()
{}

//////////////
// absolute //
//////////////
PP<RPPath> RPPath::absolute()
{
    PP<RPPath> abs_path = new RPPath();
    abs_path->path = path.absolute();
    abs_path->build();
    return abs_path;
}

///////////////
// canonical //
///////////////
string RPPath::canonical()
{
    return path.canonical();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RPPath::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
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

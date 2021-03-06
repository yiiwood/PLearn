// -*- C++ -*-

// MPIPStreamBuf.h
//
// Copyright (C) 2005 Pascal Vincent 
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
 * $Id: MPIPStreamBuf.h $ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file MPIPStreamBuf.h */


#ifndef MPIPStreamBuf_INC
#define MPIPStreamBuf_INC

#include "PStreamBuf.h"

namespace PLearn {

class Poll;
  
/** An implementation of the PStreamBuf interface using MPI communication. */
class MPIPStreamBuf: public PStreamBuf
{
    friend class PLearn::Poll;
  
private:
  
    typedef PStreamBuf inherited;
    char* mpibuf;
    streamsize mpibuf_capacity;
    char* mpibuf_p;
    streamsize mpibuf_len;
    bool reached_eof;
    streamsize max_chunksize;

    void fill_mpibuf(int msglength);

protected:
    // *********************
    // * protected options *
    // *********************
    int peerrank; //!< rank of MPI peer


public:

    MPIPStreamBuf(int peer_rank, streamsize chunksize=1024);
    virtual ~MPIPStreamBuf();

protected:

    virtual streamsize read_(char* p, streamsize n);

    //! writes exactly n characters from p (unbuffered, must flush)
    virtual void write_(const char* p, streamsize n);

};

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

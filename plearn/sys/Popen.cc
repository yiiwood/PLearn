// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2001 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "Popen.h"

#include <nspr/prio.h>
#include <nspr/prproces.h>
#include <nspr/prenv.h>
#include <plearn/base/stringutils.h>
#include <plearn/base/PrUtils.h>
#include <plearn/io/PrPStreamBuf.h>
#include <plearn/io/openString.h>


namespace PLearn {
using namespace std;

void Popen::launch(const string& program, const vector<string>& arguments, bool redirect_stderr)
{
    // Create pipes to communicate to/from child process.
    PRFileDesc* stdout_child;
    PRFileDesc* stdout_parent;
  
    if (PR_CreatePipe(&stdout_parent, &stdout_child) != PR_SUCCESS)
        PLERROR("Popen: error creating first pipe pair. (%s)",
                getPrErrorString().c_str());
    PRFileDesc* stdin_child;
    PRFileDesc* stdin_parent;

    if (PR_CreatePipe(&stdin_child, &stdin_parent) != PR_SUCCESS) {
        PR_Close(stdout_child);
        PR_Close(stdout_parent);
        PLERROR("Popen: error creating second pipe pair. (%s)",
                getPrErrorString().c_str());
    }

    PRFileDesc* stderr_child;
    PRFileDesc* stderr_parent;

    if(redirect_stderr)
        if (PR_CreatePipe(&stderr_child, &stderr_parent) != PR_SUCCESS) 
        {
            PR_Close(stderr_child);
            PR_Close(stderr_parent);
            PLERROR("Popen: error creating third (err) pipe pair. (%s)",
                    getPrErrorString().c_str());
        }

    // Set up redirection of stdin/stdout for the (future) child process.
    PRProcessAttr* process_attr = PR_NewProcessAttr();
    PR_ProcessAttrSetStdioRedirect(process_attr, PR_StandardInput, stdin_child);
    PR_ProcessAttrSetStdioRedirect(process_attr, PR_StandardOutput, stdout_child);
    if(redirect_stderr)
        PR_ProcessAttrSetStdioRedirect(process_attr, PR_StandardError, stderr_child);

    // Set up argument list for the CreateProcess call. args[0] shoud be the
    // name of the program. args[1]...arg[n] hold the actual arguments,
    // arg[n+1] is NULL.
#if !defined(_MINGW_) && !defined(WIN32)
    const char** args = new const char*[4];
    // NSPR doesn't have a way to search through the PATH when creating
    // a new process. Use /bin/sh for now to spawn the process as a workaround.
    args[0] = "/bin/sh";
    args[1] = "-c";

    string concatenated_args = program;
    for (vector<string>::const_iterator it = arguments.begin(); it != arguments.end();
         ++it)
        concatenated_args += " "+quote_string(*it);
    args[2] = concatenated_args.c_str();
    args[3] = 0;
  
    process = PR_CreateProcess("/bin/sh",
                               const_cast<char* const *>(args),
                               NULL, process_attr);
#else
    const char** args = new const char*[arguments.size()+2];
    // PR_CreateProcess on Windows goes through the PATH.
    // No workaround needed here.
    args[0] = program.c_str();
    int i = 1;
    for (vector<string>::const_iterator it = arguments.begin(); it != arguments.end();
         ++it)
        args[i++] = it->c_str();
    args[i] = 0;
  
    process = PR_CreateProcess(program.c_str(),
                               const_cast<char* const *>(args),
                               NULL, process_attr);  
#endif

    // Important: close unused files in the parent.
    PR_Close(stdin_child);
    PR_Close(stdout_child);
    if(redirect_stderr)
        PR_Close(stderr_child);
  
    delete[] args;                        
    if (!process) {
        PR_Close(stdin_parent);
        PR_Close(stdout_parent);
        if(redirect_stderr)
            PR_Close(stderr_parent);
        PLERROR("Popen: could not create subprocess for command '%s'. (%s)",
                program.c_str(), getPrErrorString().c_str());
    }
    process_alive = true;

    in = new PrPStreamBuf(stdout_parent, stdin_parent);
    in.setBufferCapacities(0, 0, 0);
    out = in;

    if(redirect_stderr)
    {    
        err= new PrPStreamBuf(stderr_parent);
        err.setBufferCapacities(0, 0, 0);
    }
}


void Popen::launch(const string& commandline, bool redirect_stderr)
{
    // Parse command line into individual argments
    string tmp_commandline = "[" + commandline + "]";
    PStream s = openString(tmp_commandline, PStream::plearn_ascii);

    vector<string> command_and_args;
    s >> command_and_args;
    const string command = command_and_args[0];
    const vector<string> args(command_and_args.begin()+1,
                              command_and_args.end());
    launch(command, args, redirect_stderr);
}


int Popen::wait()
{
    int status = 0;
    if (process_alive)
        if (PR_WaitProcess(process, &status) != PR_SUCCESS)
            PLERROR("Popen: error while waiting for subprocess to terminate. (%s)",
                    getPrErrorString().c_str());
    process_alive = false;
  
    return status;
}


Popen::~Popen()
{
    // If we get here, there's no way the caller that instantiated us will be
    // reading any more from the process at the other end of the pipe, and they
    // won't be able to retrieve the exit code either. So just make sure we
    // don't hang on the wait and that we don't leave zombie processes around.
    PR_KillProcess(process);
    wait();
}

  
vector<string> execute(const string& command, bool redirect_stderr)
{
    Popen p(command, redirect_stderr);
    vector<string> result;
    while(p.in.good())
    {
        string line = p.in.getline();
        result.push_back(line);
    }
    return result;
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

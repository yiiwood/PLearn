// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: fileutils.h,v 1.12 2004/03/10 19:54:43 tihocan Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

// This file contains useful functions for string manipulation
// that are used in the PLearn Library


/*! \file PLearnLibrary/PLearnCore/fileutils.h */

#ifndef fileutils_INC
#define fileutils_INC

#include <fstream>
#include <map>
#include <string>
#include <vector>
//#include "general.h"

// norman: set win32 functions
#ifdef WIN32
#include <sys/types.h>
#define stat _stat
#endif

namespace PLearn {
using namespace std;

  //! returns the absolute path to the current working directory as a string
  string getcwd();

  //! change current directory
  int chdir(const string& path);

  //! returns the absolute path of the (possibly relative) specified path.
  //! if it's a directory, then there will be a trailing slash.
  string abspath(const string& path);
  
  //!  returns true if the given path points to an existing regular file or directory 
  bool pathexists(const string& path);

  //!  returns true if the given path is an existing directory (or a symbolic link pointing to a directory)
  bool isdir(const string& path);

  //!  returns true if the given path is an existing regular file (or a symbolic link pointing to a file)
  bool isfile(const string& path);

  //! returns the time of last modification of file (or 0 if file does not exist).
  time_t mtime(const string& path);

/*!     Returns a list of all entries in the given directory (omitting entries "." and "..")
    If the direcotry cannot be opened an error is issued.
    The returned entries are not full paths.
*/
  vector<string> lsdir(const string& dirpath);

  //!  Same as lsdir, except dirpath is prepended to the entries' names.
  vector<string> lsdir_fullpath(const string& dirpath);

/*!     Forces directory creation if it doesn't already exist. 
    (also creates any missing directory along its path)
    Return value indicates success (true) or failure (false).
    If the directory already exists, true is returned.
*/
  bool force_mkdir(const string& dirname);

//! Extracts the directory part of the filepath and calls force_mkdir
//! Calls PLERROR in case of failure.
void force_mkdir_for_file(const string& filepath);

/*!     Forces removal of directory and all its content
    Return value indicates success (true) or failure (false)
    If the directory does not exist, false is returned.
*/
  bool force_rmdir(const string& dirname);

  //! Returns the length of a file, measured in bytes.
  long filesize(const string& filename);

  //! calls system with cp -R to recursively copy source to destination
  void cp(const string& srcpath, const string& destpath);

  //! calls system rm command with string file as parameters
  void rm(const string& file);

  //! calls system mv command with string file as parameters
  void mv(const string& file);

  //! calls system mv command with string file as parameters
  //! will not prompt before overwriting
  void mvforce(const string& file);

  //! trivial unix touch
  void touch(const string& file);

  //! Reads while the characters read exactly match those in s
  //! Will throw a PLERROR exception as soon as it doesn't match
  void readWhileMatches(istream& in, const string& s);

  //! skips everything until '\n' (also consumes the '\n')
  void skipRestOfLine(istream& in);

  //! will skip all blanks (white space, newline and #-style comments) 
  //! Next character read will be first "non-blank"
  void skipBlanksAndComments(istream& in);

  //! returns the next non blank line (#-style comments are considered blank)
  void getNextNonBlankLine(istream& in, string& line);

  //! Will return the number of non-blank lines of file
  //! #-style comments are considered blank
  int countNonBlankLinesOfFile(const string& filename);

  //! same as PStream's method smartReadUntilNext, but for istream
  int smartReadUntilNext(istream& in, string stoppingsymbols, string& characters_read);
  
  //! peeks the first char after removal of blanks
  inline char peekAfterSkipBlanks(istream& in) { while(isspace(in.get())); in.unget();return in.peek(); }

  //! gets the first char after removal of blanks
  inline char getAfterSkipBlanks(istream& in) { char c; while(isspace(c = in.get())); return c; }

  //! Returns a temporary file (or directory) name suitable
  //! for a unique (one time) use.
  string newFilename(const string directory="/tmp/", const string prefix="", bool is_directory=false);

  string makeFileNameValid(const string& filename);

  //! returns "./"+filename if filename is relative to current dir
  string makeExplicitPath(const string& filename);

  //! Returns the whole content of the file as a string
  string loadFileAsString(const string& filepath);

  //! Writes the raw string into the given file
  //! Intermediate directories in filepath are created if necessary
  void saveStringInFile(const string& filepath, const string& text);

  //! Will return the text, macro processed, with each instance of ${varname} in the text that corresponds to a key in the given map 
  //! replaced by its associated value. 
  //! Also every $DEFINE{varname=... } in the text will add a new varname entry in the map.  (The DEFINE macro will be discarded)
  //! Also every $INCLUDE{filepath} will be replaced in place by the text of the file it includes
  string readAndMacroProcess(istream& in, map<string, string>& variables);

//! Same as readAndMacroProcess, but takes a filename instead of an istream
//! The following variables are automatically set from the filepath: FILEPATH DIRPATH FILENAME FILEBASE FILEEXT 
/*! Ex: if the absolute path to filepath is /home/me/foo.plearn
  Then we'll get:
  FILEPATH = "/home/me/foo.plearn"
  DIRPATH  = "/home/me"
  FILENAME = "foo.plearn"
  FILEBASE = "foo"
  FILEEXT  = ".plearn"
*/
string readFileAndMacroProcess(const string& filepath, map<string, string>& variables);

inline string readFileAndMacroProcess(const string& filepath)
{
  map<string, string> variables;
  return readFileAndMacroProcess(filepath, variables);
}

#ifdef WIN32
#undef stat
#endif

} // end of namespace PLearn


#endif

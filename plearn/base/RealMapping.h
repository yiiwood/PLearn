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
   * $Id: RealMapping.h,v 1.5 2002/10/24 20:17:12 zouave Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef RealMapping_INC
#define RealMapping_INC

#include "general.h"
#include "Object.h"
#include "TMat.h"

namespace PLearn <%
using namespace std;

  //! represents a real range: i.e. one of ]low,high[ ; [low,high[; [low,high]; ]low,high]
  class RealRange
  {
  public:
    real low;
    real high;
    char leftbracket; // either '[' (inclusive left) or ']' (exclusive left)
    char rightbracket; // // either '[' (exclusive right) or ']' (inclusive right)

  public:
    RealRange(): // default construvtor
      low(0), high(0), leftbracket(']'), rightbracket('[')
    {}
      

    RealRange(char leftbracket_, real low_, real high_, char rightbracket_):
      low(low_), high(high_), leftbracket(leftbracket_), rightbracket(rightbracket_)
    { checkbrackets(); } 

    real span(){return abs(high-low);}
    
    void checkbrackets() const
    {
      if( (leftbracket!='[' && leftbracket!=']') || (rightbracket!='[' && rightbracket!=']') )
        PLERROR("In RealRange: Brackets must be either '[' or ']'"); 
    }

    bool contains(real val)
    { return (val>=low) && (val<=high) && (val!=low || leftbracket=='[') && (val!=high || rightbracket==']'); }

    void print(ostream& out) const
    { out << leftbracket << low << ' ' << high << rightbracket; }

    void write(ostream& out) const
    { out << leftbracket << low << ' ' << high << rightbracket; }

    void read(istream& in)
    { in >> leftbracket >> low >> high >> rightbracket; checkbrackets(); }
  };

  inline void write(ostream& out, const RealRange& range) { range.write(out); }
  inline ostream& operator<<(ostream& out, const RealRange& range) { range.print(out); return out; } 
  inline void read(istream& in, RealRange& range) { range.read(in); }
  

  class RealMapping: public Object
  {
  public:
    typedef TVec< pair<RealRange, real> > mapping_t;
    mapping_t mapping; // defines mapping from real ranges to values
    real missing_mapsto; // value to which to map missing values (can be missing value)
    bool keep_other_as_is; // if true, values not in mapping are left as is, otherwise they're mappred to other_mapsto
    real other_mapsto; // value to which to map values not inmapping, if keep_other_as_is is false 
    
  public:
    DECLARE_NAME_AND_DEEPCOPY(RealMapping);
    
    RealMapping()
      :missing_mapsto(MISSING_VALUE),
       keep_other_as_is(true),
       other_mapsto(MISSING_VALUE)
    {}

    int size() const { return mapping.length(); }
    int length() const { return mapping.length(); }

    //! Removes all entries in mapping.  Does not change other params.
    inline void clear() { mapping.clear(); }
    
    void addMapping(const RealRange& range, real val)
    { mapping.push_back(make_pair(range,val)); }

    //! Set mapping for missing value (by default it maps to MISSING_VALUE)
    void setMappingForMissing(real what_missing_mapsto)
    { missing_mapsto = what_missing_mapsto; }

    //! Set mapping for any other value (by default it is kept as is)
    void setMappingForOther(real what_other_mapsto)
    { keep_other_as_is=false; other_mapsto = what_other_mapsto; }

    void keepOtherAsIs() 
    { keep_other_as_is=true; }

    // returns the mapped value corresponding to val
    real map(real val) const;

    // returns the number of the bin in which 'val' falls
    int binnumber(real val) const;

    // transforms v by applying the mapping on all its elements
    void transform(const Vec& v) const;

    pair<RealRange, real>& lastMapping() 
    { return mapping.lastElement(); }

    virtual void print(ostream& out) const;
    virtual void write(ostream& out) const;
    virtual void read(istream& in);
  };

  DECLARE_OBJECT_PTR(RealMapping);

%> // end of namespace PLearn

#endif

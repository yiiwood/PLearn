// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003 Olivier Delalleau
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
   * $Id: SortRowsVMatrix.cc,v 1.1 2003/12/05 18:55:01 tihocan Exp $
   ******************************************************* */

#include "SubVMatrix.h"
#include "SortRowsVMatrix.h"

namespace PLearn <%
using namespace std;

/** SortRowsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SortRowsVMatrix, 
    "Sort the samples of a VMatrix according to one (or more) given columns.\n"
    "The implementation is not efficient at all, feel free to improve it !",
    "NO HELP");

SortRowsVMatrix::SortRowsVMatrix() 
  : increasing_order(1)
{
  // Default = no sorting.
  sort_columns.resize(0);
}

void SortRowsVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "sort_columns", &SortRowsVMatrix::sort_columns, OptionBase::buildoption,
        "    the column(s) that must be sorted (the first one is the first criterion)");
    
    declareOption(ol, "increasing_order", &SortRowsVMatrix::increasing_order, OptionBase::buildoption,
        "    if set to 1, the data will be sorted in increasing order)");
    
    inherited::declareOptions(ol);
}

void SortRowsVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  deepCopyField(sort_columns, copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
}

///////////
// build //
///////////
void SortRowsVMatrix::build()
{
  inherited::build();
  build_();
}

//////////////
// sortRows //
//////////////
void SortRowsVMatrix::sortRows(VMat& m, TVec<int>& indices, TVec<int>& sort_columns, int istart, int iend, int colstart, bool increasing_order) {
  real best;
  real jval;
  int tmp;
  bool better;
  if (sort_columns.size() > colstart) {
    int col = sort_columns[colstart]; // The current column used to perform sorting.
    for (int i = istart; i <= iend-1; i++) {
      // Let's look for the i-th element of our result.
      best = m(indices[i],col);
      for (int j = i+1; j <= iend; j++) {
        better = false;
        jval = m(indices[j],col);
        if (increasing_order && jval < best) {
          better = true;
        } else if (!increasing_order && jval > best) {
          better = true;
        }
        if (better) {
          // Swap i and j.
          tmp = indices[j];
          indices[j] = indices[i];
          indices[i] = tmp;
          best = jval;
        }
      }
    }
    // At this point, we have only sorted according to one column.
    if (sort_columns.length() > colstart + 1) {
      // There are other sorting criterions.
      // Let's find where we need to apply them.
      int i = istart;
      real val;
      while (i <= iend - 1) {
        val = m(indices[i],col);
        int j = i+1;
        while (j <= iend && m(indices[j],col) == val)
          j++;
        j--;
        if (j > i) {
          // There are consecutive elements with the same value for the sorting
          // criterion, thus we must use the other criteria to sort them correctly.
          sortRows(m, indices, sort_columns, i, j, colstart + 1, increasing_order);
        }
        i = j+1;
      }
    }
  }
}

////////////
// build_ //
////////////
void SortRowsVMatrix::build_()
{
  // Construct the indices vector.
  indices = TVec<int>(0, distr.length()-1, 1);
  sortRows(distr, indices, sort_columns, 0, distr->length()-1, 0, increasing_order);
  inherited::build(); // Since we have just changed the indices.
  /*real best;
  real jval;
  int tmp;
  bool better;
  if (sort_columns.size() > 0) {
    int col = sort_columns[0]; // The first column used to perform sorting.
    for (int i = 0; i < n-1; i++) {
      // Let's look for the i-th element of our result.
      best = distr(indices[i],col);
      for (int j = i+1; j < n; j++) {
        better = false;
        jval = distr(indices[j],col);
        if (increasing_order && jval < best) {
          better = true;
        } else if (!increasing_order && jval > best) {
          better = true;
        }
        if (better) {
          // Swap i and j.
          tmp = indices[j];
          indices[j] = indices[i];
          indices[i] = tmp;
          best = jval;
        }
      }
    }
    // At this point, we have only sorted according to the first column.
    if (sort_columns.length() > 1) {
      // There are other sorting criterions.
      // Let's find where we need to apply them.
      int i = 0;
      real val;
      while (i < n-1) {
        val = distr(indices[i],col);
        int j = i+1;
        while (j < n && distr(indices[j],col) == val)
          j++;
        j--;
        if (j > i) {
          // There are consecutive elements with the same value for the sorting
          // criterion, thus we must use the other criteria to sort them correctly.
          // For this, we use recursively a new SortRowsVMatrix with the remaining
          // criteria.
          VMat sub = new SubVMatrix(this, i, 0, j-i+1, distr.width());
          PP<SortRowsVMatrix> sortrow = new SortRowsVMatrix();
          sortrow->distr = sub;
          sortrow->sort_columns = sort_columns.subVec(1, sort_columns.length() - 1);
          sortrow->increasing_order = this->increasing_order;
          sortrow->build();
          TVec<int> tmp_storage(j-i+1);
          for (int k = 0; k < j-i+1; k++) {
            tmp_storage[k] = indices[i + sortrow->indices[k]];
          }
          for (int k = 0; k < j-i+1; k++) {
            indices[i + k] = tmp_storage[k];
          }
        }
        i = j+1;
        // Note that we technically should call inherited::build after we
        // changed the indices, but it's not necessary here.
      }
    }
    // We have changed the indices, so we must rebuild.
    inherited::build();
  } */
}

%> // end of namespcae PLearn

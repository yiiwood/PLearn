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
   * $Id: Kernel.cc,v 1.26 2004/05/17 15:12:54 ouimema Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "Kernel.h"
#include "ProgressBar.h"

namespace PLearn {
using namespace std;

using namespace std;
  
PLEARN_IMPLEMENT_ABSTRACT_OBJECT(Kernel, "ONE LINE DESCR", "NO HELP");
Kernel::~Kernel() {}

////////////
// Kernel //
////////////
Kernel::Kernel(bool is__symmetric)
: data_inputsize(-1),
  n_examples(-1),
  is_symmetric(is__symmetric),
  report_progress(0)
{}

////////////////////
// declareOptions //
////////////////////
void Kernel::declareOptions(OptionList &ol)
{

  // Build options.

  declareOption(ol, "is_symmetric", &Kernel::is_symmetric, OptionBase::buildoption,
                "Whether this kernel is symmetric or not.");
  
  declareOption(ol, "report_progress", &Kernel::report_progress, OptionBase::buildoption,
                "If set to 1, a progress bar will be displayed when computing the Gram matrix,\n"
                "or for other possibly costly operations.");
  
  declareOption(ol, "specify_dataset", &Kernel::specify_dataset, OptionBase::buildoption,
                "If set, then setDataForKernelMatrix will be called with this dataset at build time");

  // Learnt options.
  
  declareOption(ol, "data_inputsize", &Kernel::data_inputsize, OptionBase::learntoption,
                "The inputsize of 'data' (if -1, is set to data.width()).");
  
  declareOption(ol, "n_examples", &Kernel::n_examples, OptionBase::learntoption,
                "The number of examples in 'data'.");
  
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void Kernel::build() {
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void Kernel::build_() {
  if (specify_dataset) {
    this->setDataForKernelMatrix(specify_dataset);
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void Kernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(data, copies);
  deepCopyField(specify_dataset, copies);
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void Kernel::setDataForKernelMatrix(VMat the_data)
{ 
  data = the_data; 
  if (data) {
    data_inputsize = data->inputsize();
    if (data_inputsize == -1) {
      // Default value when no inputsize is specified.
      data_inputsize = data->width();
    }
    n_examples = data->length();
  } else {
    data_inputsize = 0;
    n_examples = 0;
  }
}

////////////////////////////
// addDataForKernelMatrix //
////////////////////////////
void Kernel::addDataForKernelMatrix(const Vec& newRow)
{
  try{
    data->appendRow(newRow);
  }
  catch(const PLearnError& e){
    PLERROR("Kernel::addDataForKernelMatrix: if one intends to use this method,\n"
            "he must provide a data matrix for which the appendRow method is\n"
            "implemented.");
  }
}

//////////////
// evaluate //
//////////////
real Kernel::evaluate_i_j(int i, int j) const
{ return evaluate(data.getSubRow(i,data_inputsize),data.getSubRow(j,data_inputsize)); }


real Kernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const 
{ return evaluate(data.getSubRow(i,data_inputsize),x); }


real Kernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
{ 
  if(is_symmetric)
    return evaluate_i_x(i,x,squared_norm_of_x);
  else
    return evaluate(x,data.getSubRow(i,data_inputsize));
}

real Kernel::evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x, bool first_time) {
  return evaluate_i_x(i, x, squared_norm_of_x);
}

real Kernel::evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x, bool first_time) {
  return evaluate_x_i(x, i, squared_norm_of_x);
}

//////////////////////
// evaluate_all_i_x //
//////////////////////
void Kernel::evaluate_all_i_x(const Vec& x, Vec& k_xi_x, real squared_norm_of_x) {
  k_xi_x[0] = evaluate_i_x_again(0, x, squared_norm_of_x, true);
  for (int i = 1; i < k_xi_x.length(); i++) {
    k_xi_x[i] = evaluate_i_x_again(i, x, squared_norm_of_x);
  }
}

//////////////////////
// evaluate_all_x_i //
//////////////////////
void Kernel::evaluate_all_x_i(const Vec& x, Vec& k_x_xi, real squared_norm_of_x) {
  k_x_xi[0] = evaluate_x_i_again(x, 0, squared_norm_of_x, true);
  for (int i = 1; i < k_x_xi.length(); i++) {
    k_x_xi[i] = evaluate_x_i_again(x, i, squared_norm_of_x);
  }
}

///////////////////////
// computeGramMatrix //
///////////////////////
void Kernel::computeGramMatrix(Mat K) const
{
  if (!data) PLERROR("Kernel::computeGramMatrix should be called only after setDataForKernelMatrix");
  int l=data->length();
  int m=K.mod();
  ProgressBar* pb = 0;
  int count = 0;
  if (report_progress) {
    pb = new ProgressBar("Computing Gram matrix for " + classname(), (l * (l + 1)) / 2);
  }
  for (int i=0;i<l;i++)
  {
    real* Ki = K[i];
    real* Kji_ = &K[0][i];
    for (int j=0;j<=i;j++,Kji_+=m)
    {
      real Kij = evaluate_i_j(i,j);
      Ki[j]=Kij;
      if (j<i)
        *Kji_ =Kij;
    }
    if (pb) {
      count += i + 1;
      pb->update(count);
    }
  }
  if (pb) {
    delete pb;
  }
}

///////////////////
// setParameters //
///////////////////
void Kernel::setParameters(Vec paramvec)
{ PLERROR("setParameters(Vec paramvec) not implemented for this kernel"); }

///////////////////
// getParameters //
///////////////////
Vec Kernel::getParameters() const
{ return Vec(); }

/////////////
// hasData //
/////////////
bool Kernel::hasData() {
  return data;
}

///////////
// apply //
///////////
void Kernel::apply(VMat m1, VMat m2, Mat& result) const
{
  result.resize(m1->length(), m2->length());
  int m1w = m1->inputsize();
  if (m1w == -1) { // No inputsize specified: using width instead.
    m1w = m1->width();
  }
  int m2w = m2->inputsize();
  if (m2w == -1) {
    m2w = m2->width();
  }
  Vec m1_i(m1w);
  Vec m2_j(m2w);
  ProgressBar* pb = 0;
  bool easy_case = (is_symmetric && m1 == m2);
  int l1 = m1->length();
  int l2 = m2->length();
  if (report_progress) {
    int nb_steps;
    if (easy_case) {
      nb_steps = (l1 * (l1 + 1)) / 2;
    } else {
      nb_steps = l1 * l2;
    }
    pb = new ProgressBar("Applying " + classname() + " to two matrices", nb_steps);
  }
  int count = 0;
  if(easy_case)
    {
      for(int i=0; i<m1->length(); i++)
        {
          m1->getSubRow(i,0,m1_i);
          for(int j=0; j<=i; j++)
            {
              m2->getSubRow(j,0,m2_j);
              real val = evaluate(m1_i,m2_j);
              result(i,j) = val;
              result(j,i) = val;
            }
          if (pb) {
            count += i + 1;
            pb->update(count);
          }
        }
    }
  else
    {
      for(int i=0; i<m1->length(); i++)
        {
          m1->getSubRow(i,0,m1_i);
          for(int j=0; j<m2->length(); j++)
            {
              m2->getSubRow(j,0,m2_j);
              result(i,j) = evaluate(m1_i,m2_j);
            }
          if (pb) {
            count += l2;
            pb->update(count);
          }
        }
    }
  if (pb)
    delete pb;
}


void Kernel::apply(VMat m, const Vec& x, Vec& result) const
{
  result.resize(m->length());
  int mw = m->inputsize();
  if (mw == -1) { // No inputsize specified: using width instead.
    mw = m->width();
  }
  Vec m_i(mw);
  for(int i=0; i<m->length(); i++)
    {
      m->getSubRow(i,0,m_i);
      result[i] = evaluate(m_i,x);
    }
}


void Kernel::apply(Vec x, VMat m, Vec& result) const
{
  result.resize(m->length());
  int mw = m->inputsize();
  if (mw == -1) { // No inputsize specified: using width instead.
    mw = m->width();
  }
  Vec m_i(mw);
  for(int i=0; i<m->length(); i++)
    {
      m->getSubRow(i,0,m_i);
      result[i] = evaluate(x,m_i);
    }
}


Mat Kernel::apply(VMat m1, VMat m2) const
{
  Mat result;
  apply(m1,m2,result);
  return result;
}

//////////
// test //
//////////
real Kernel::test(VMat d, real threshold, real sameness_below_threshold, real sameness_above_threshold) const
{
  int nerrors = 0;
  int inputsize = (d->width()-1)/2;
  for(int i=0; i<d->length(); i++)
    {
      Vec inputs = d(i);
      Vec input1 = inputs.subVec(0,inputsize);
      Vec input2 = inputs.subVec(inputsize,inputsize);
      real sameness = inputs[inputs.length()-1];
      real kernelvalue = evaluate(input1,input2);
      cerr << "[" << kernelvalue << " " << sameness << "]\n";
      if(kernelvalue<threshold)
        {
          if(sameness==sameness_above_threshold)
            nerrors++;
        }
      else // kernelvalue>=threshold
        {
          if(sameness==sameness_below_threshold)
            nerrors++;
        }
    }
  return real(nerrors)/d->length();
}


//////////////////////////////////////////////
// computeKNNeighbourMatrixFromDistanceMatrix //
//////////////////////////////////////////////
Mat Kernel::computeKNNeighbourMatrixFromDistanceMatrix(const Mat& D, int knn, bool insure_self_first_neighbour, bool report_progress)
{
  int npoints = D.length();
  Mat neighbours(npoints, knn);  
  Mat tmpsort(npoints,2);

  ProgressBar* pb = 0;
  if (report_progress) {
    pb = new ProgressBar("Computing neighbour matrix", npoints);
  }
  
    //for(int i=0; i<2; i++)
  for(int i=0; i<npoints; i++)
    {
      for(int j=0; j<npoints; j++)
        {
          tmpsort(j,0) = D(i,j);
          tmpsort(j,1) = j;
        }
      if(insure_self_first_neighbour)
        tmpsort(i,0) = -FLT_MAX;

      partialSortRows(tmpsort, knn);
      neighbours(i) << tmpsort.column(1).subMatRows(0,knn);
      if (pb)
        pb->update(i);
    }
  if (pb)
    delete pb;
  return neighbours;
}

//////////////////////////////////////////////
// computeNeighbourMatrixFromDistanceMatrix //
//////////////////////////////////////////////
//  You should use computeKNNeighbourMatrixFromDistanceMatrix instead.
Mat Kernel::computeNeighbourMatrixFromDistanceMatrix(const Mat& D, bool insure_self_first_neighbour, bool report_progress)
{
  // TODO Make it possible to only compute the k nearest neighbours.
  int npoints = D.length();
  Mat neighbours(npoints, npoints);  
  Mat tmpsort(npoints,2);

  ProgressBar* pb = 0;
  if (report_progress) {
    pb = new ProgressBar("Computing neighbour matrix", npoints);
  }
  
    //for(int i=0; i<2; i++)
  for(int i=0; i<npoints; i++)
    {
      for(int j=0; j<npoints; j++)
        {
          tmpsort(j,0) = D(i,j);
          tmpsort(j,1) = j;
        }
      if(insure_self_first_neighbour)
        tmpsort(i,0) = -FLT_MAX;

      sortRows(tmpsort);
      neighbours(i) << tmpsort.column(1);
      if (pb)
        pb->update(i);
    }
  if (pb)
    delete pb;
  return neighbours;
}


Mat Kernel::estimateHistograms(VMat d, real sameness_threshold, real minval, real maxval, int nbins) const
{
  real binwidth = (maxval-minval)/nbins;
  int inputsize = (d->width()-1)/2;
  Mat histo(2,nbins);
  Vec histo_below = histo(0);
  Vec histo_above = histo(1);
  int nbelow=0;
  int nabove=0;
  for(int i=0; i<d->length(); i++)
    {
      Vec inputs = d(i);
      Vec input1 = inputs.subVec(0,inputsize);
      Vec input2 = inputs.subVec(inputsize,inputsize);
      real sameness = inputs[inputs.length()-1];
      real kernelvalue = evaluate(input1,input2);
      if(kernelvalue>=minval && kernelvalue<maxval)
        {
          int binindex = int((kernelvalue-minval)/binwidth);
          if(sameness<sameness_threshold)
            {
              histo_below[binindex]++;
              nbelow++;
            }
          else
            {
              histo_above[binindex]++;
              nabove++;
            }
        }
    }
  histo_below /= real(nbelow);
  histo_above /= real(nabove);
  return histo;
}


Mat Kernel::estimateHistograms(Mat input_and_class, real minval, real maxval, int nbins) const
{
  real binwidth = (maxval-minval)/nbins;
  int inputsize = input_and_class.width()-1;
  Mat inputs = input_and_class.subMatColumns(0,inputsize);
  Mat classes = input_and_class.column(inputsize);
  Mat histo(4,nbins);
  Vec histo_mean_same = histo(0);
  Vec histo_mean_other = histo(1);
  Vec histo_min_same = histo(2);
  Vec histo_min_other = histo(3);

  for(int i=0; i<inputs.length(); i++)
    {
      Vec input = inputs(i);
      real input_class = classes(i,0);
      real sameclass_meandist = 0.0;
      real otherclass_meandist = 0.0;
      real sameclass_mindist = FLT_MAX;
      real otherclass_mindist = FLT_MAX;
      for(int j=0; j<inputs.length(); j++)
        if(j!=i)
          {
            real dist = evaluate(input, inputs(j));
            if(classes(j,0)==input_class)
              {
                sameclass_meandist += dist;
                if(dist<sameclass_mindist)
                  sameclass_mindist = dist;
              }
            else
              {
                otherclass_meandist += dist;
                if(dist<otherclass_mindist)
                  otherclass_mindist = dist;
              }
          }
      sameclass_meandist /= (inputs.length()-1);
      otherclass_meandist /= (inputs.length()-1);      
      if(sameclass_meandist>=minval && sameclass_meandist<maxval)
        histo_mean_same[int((sameclass_meandist-minval)/binwidth)]++;
      if(sameclass_mindist>=minval && sameclass_mindist<maxval)
        histo_min_same[int((sameclass_mindist-minval)/binwidth)]++;
      if(otherclass_meandist>=minval && otherclass_meandist<maxval)
        histo_mean_other[int((otherclass_meandist-minval)/binwidth)]++;
      if(otherclass_mindist>=minval && otherclass_mindist<maxval)
        histo_min_other[int((otherclass_mindist-minval)/binwidth)]++;
    }
  histo_mean_same /= sum(histo_mean_same);
  histo_min_same /= sum(histo_min_same);
  histo_mean_other /= sum(histo_mean_other);
  histo_min_other /= sum(histo_min_other);
  return histo;
}

/*
void
Kernel::oldwrite(ostream& out) const
{
	writeHeader(out,"Kernel");
	writeField(out,"is_symmetric",is_symmetric);
	writeFooter(out,"Kernel");
}


void
Kernel::oldread(istream& in)
{
	readHeader(in,"Kernel");
	readField(in,"is_symmetric",is_symmetric);
	readFooter(in,"Kernel");
}
*/

// last column of data is supposed to be a classnum
// returns a matrix of (index1, index2, distance)
Mat findClosestPairsOfDifferentClass(int k, VMat data, Ker dist)
{
  Mat result(k,3);
  real maxdistinlist = -FLT_MAX;
  int posofmaxdistinlist = -1;
  int kk=0; // number of pairs already in list
  Vec rowi(data.width());
  Vec inputi = rowi.subVec(0,rowi.length()-1);
  real& targeti = rowi[rowi.length()-1];
  Vec rowj(data.width());
  Vec inputj = rowj.subVec(0,rowj.length()-1);
  real& targetj = rowj[rowj.length()-1];
  for(int i=0; i<data.length(); i++)
  {
    data->getRow(i,rowi);
    for(int j=0; j<data.length(); j++)
    {
      data->getRow(j,rowj);
      if(targeti!=targetj)
      {
        real d = dist(inputi,inputj);
        if(kk<k)
        {
          result(kk,0) = i;
          result(kk,1) = j;
          result(kk,2) = d;
          if(d>maxdistinlist)
          {
            maxdistinlist = d;
            posofmaxdistinlist = kk;
          }
          kk++;
        }
        else if(d<maxdistinlist)
        {
          result(posofmaxdistinlist,0) = i;
          result(posofmaxdistinlist,1) = j;
          result(posofmaxdistinlist,2) = d;
          posofmaxdistinlist = argmax(result.column(2));
          maxdistinlist = result(posofmaxdistinlist,2);
        }
      }
    }
  }
  sortRows(result, 2);//use partialSortRows instead
  return result;
}

} // end of namespace PLearn


// -*- C++ -*-
// convolutions.cc

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of
// Montreal
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

/*! \file plearn/math/convolutions.cc */


#include "convolutions.h"

namespace PLearn {
using namespace std;

//! Convolve a source signal of length NS with a kernel of length NK
//! with steps S, and put result in a destination signal which should
//! be of length NS-NK+1.
//! The destination signal is
//!    dest_signal[i] = sum_{j=0}^{NK-1} source_signal[i*step+j]*kernel[j]
void convolve1D(const Vec& source_signal, const Vec& kernel,
                const Vec& dest_signal, int step, bool accumulate)
{
    int nk=kernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    int ns=source_signal.length();
    if (step<1)
        PLERROR("convolve1D: step (%d) should be a positive integer\n",step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("convolve1D: source_signal.length() (%d) should equal %d:\n"
                "step (%d) * (dest_signal.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
#endif
    if (!accumulate)
        dest_signal.clear();
    real* s=source_signal.data();
    real* k=kernel.data();
    real* d=dest_signal.data();
    for (int i=0;i<nd;i++,s+=step)
    {
        real somme=0;
        for (int j=0;j<nk;j++)
            somme += s[j]*k[j];
        d[i]+=somme;
    }
}

//! Back-convolve INTO a "source" signal of length NS with a kernel of length
//! NK and FROM a "destination" signal which should be of length NS-NK+1
//! This is EXACTLY the TRANSPOSE operation of a convolve1D with the same
//! arguments, with computations flowing in the other direction.
//! for i=0 to nd-1:
//!   for j=0 to nk-1:
//!    source_signal[i*step+j] += dest_signal[i]*kernel[j]
//! If the accumulate flag is not set, then source_signal is first cleared.
//! N.B. THIS IS THE SAME AS COMPUTING dC/dsource_signal (into the
//! source_signal argument), GIVEN dC/ddest_signal, i.e. this function
//! does part of the work done by convolve1Dbackprop.
void backConvolve1D(const Vec& source_signal, const Vec& kernel,
                    const Vec& dest_signal, int step, bool accumulate)
{
    int nk=kernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    int ns=source_signal.length();
    if (step<1)
        PLERROR("backConvolve1D: step (%d) should be a positive integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("backConvolve1D: source_signal.length() (%d) should equal %d:\n"
                "step (%d) * (dest_signal.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
#endif
    if (!accumulate)
        source_signal.clear();
    real* s=source_signal.data();
    real* k=kernel.data();
    real* d=dest_signal.data();
    for (int i=0;i<nd;i++,s+=step)
    {
        //! for i=0 to nd-1:
        //!   for j=0 to nk-1:
        //!    source_signal[i*step+j] += dest_signal[i]*kernel[j]
        real di=d[i];
        for (int j=0;j<nk;j++)
            s[j] += di*k[j];
    }
}


//! Increment dC/dsource_signal and dC/dkernel, given dC/ddest_signal, with
//! dest_signal computed as per convolve1D(source_signal, kernel, dest_signal):
//!    dC/dsource_signal[k] += sum_{j=0}^{NK-1} 1_{k>=j && k-j<ND} dC_ddest_signal[k-j]*kernel[j]
//!    dC/dkernel[j] += sum_{k=0}^{ND-1} 1_{k>=j && k-j<ND} dC_ddest_signal[k-j]*source_signal[k]
//! (consider the equivalence: k = i+j)
void convolve1Dbackprop(const Vec& source_signal, const Vec& kernel,
                        const Vec& dC_ddest_signal,
                        const Vec& dC_dsource_signal, const Vec& dC_dkernel,
                        int step, bool accumulate)
{
    int nk=kernel.length();
    int nd=dC_ddest_signal.length();
#ifdef BOUNDCHECK
    int ns=source_signal.length();
    if (step<1)
        PLERROR("convolve1Dbackprop: step (%d) should be a positive integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("convolve1Dbackprop: source_signal.length() (%d) should"
                " equal %d:\n"
                "step (%d) * (dC_ddest_signal.length() (%d) - 1) +"
                " kernel.length() (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
    if (dC_dsource_signal.length()!=ns)
        PLERROR("convolve1Dbackprop: source_signal.length() (%d) should"
                " equal:\n"
                "dC_dsource_signal.length() (%d)\n",
                ns,dC_dsource_signal.length());
    if (dC_dkernel.length()!=nk)
        PLERROR("convolve1Dbackprop: kernel.length() (%d) should equal:\n"
                " dC_dkernel.length() (%d)\n",
                nk,dC_dkernel.length());
#endif
    if (!accumulate)
    {
        dC_dsource_signal.clear();
        dC_dkernel.clear();
    }
    real* s=source_signal.data();
    real* dCds=dC_dsource_signal.data();
    real* k=kernel.data();
    real* dCdk=dC_dkernel.data();
    real* dCdd=dC_ddest_signal.data();

    //! for i=0 to nd-1:
    //!   for j=0 to nk-1:
    //!     dC_dsource_signal[i*step+j] += dC_ddest_signal[i]*kernel[j]
    //!     dC_dkernel[j] += dC_ddest_signal[i]*source_signal[i*step+j]
    for (int i=0;i<nd;i++,dCds+=step,s+=step)
    {
        real di=dCdd[i];
        for (int j=0;j<nk;j++)
        {
            dCds[j] += di*k[j];
            dCdk[j] += di*s[j];
        }
    }
}

//! Same as above, but increments only dC/dkernel, not dC/dsource_signal
//!    dC/dkernel[j] += sum_{k=0}^{ND-1} 1_{k>=j && k-j<ND} dC_ddest_signal[k-j]*source_signal[k]
//! (consider the equivalence: k = i+j)
void convolve1Dbackprop(const Vec& source_signal,
                        const Vec& dC_ddest_signal,
                        const Vec& dC_dkernel,
                        int step, bool accumulate)
{
    int nk=dC_dkernel.length();
    int nd=dC_ddest_signal.length();
#ifdef BOUNDCHECK
    int ns=source_signal.length();
    if (step<1)
        PLERROR("convolve1Dbackprop: step (%d) should be a positive integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("convolve1Dbackprop: source_signal.length() (%d) should"
                " equal %d:\n"
                "step (%d) * (dC_ddest_signal.length() (%d) - 1) +"
                " dC_dkernel.length() (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
#endif
    if (!accumulate)
        dC_dkernel.clear();

    real* s=source_signal.data();
    real* dCdk=dC_dkernel.data();
    real* dCdd=dC_ddest_signal.data();

    //! for i=0 to nd-1:
    //!   for j=0 to nk-1:
    //!     dC_dkernel[j] += dC_ddest_signal[i]*source_signal[i*step+j]
    for (int i=0;i<nd;i++,s+=step)
    {
        real di=dCdd[i];
        for (int j=0;j<nk;j++)
            dCdk[j] += di*s[j];
    }
}


//! Increment dC/ddest_signal and dC/dkernel, given dC/ddest_signal, with
//! source_signal computed as per
//! backConvolve1D(source_signal, kernel, dest_signal):
//!    dC/ddest_signal[i] += sum_{j=0}^{NK-1} dC_dsource_signal[i+j]*kernel[j]
//!    dC/dkernel[j] += sum_{i=0}^{ND-1} dC_dsource_signal[i+j]*dest_signal[i]
void backConvolve1Dbackprop(const Vec& kernel, const Vec& dest_signal,
                            const Vec& dC_ddest_signal,
                            const Vec& dC_dsource_signal,
                            const Vec& dC_dkernel,
                            int step, bool accumulate)
{
    int nk=kernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    int ns=dC_dsource_signal.length();
    if (step<1)
        PLERROR("backConvolve1Dbackprop: step (%d) should be a positive"
                " integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("backConvolve1Dbackprop: dC_dsource_signal.length() (%d)\n"
                "should equal %d:\n"
                "step (%d) * (dest_signal.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
    if (dC_ddest_signal.length()!=nd)
        PLERROR("backConvolve1Dbackprop: dest_signal.length() (%d) should"
                " equal:\n"
                "dC_ddest_signal.length() (%d)\n",
                nd,dC_ddest_signal.length());
    if (dC_dkernel.length()!=nk)
        PLERROR("backConvolve1Dbackprop: kernel.length() (%d) should equal:\n"
                " dC_dkernel.length() (%d)\n",
                nk,dC_dkernel.length());
#endif
    if (!accumulate)
    {
        dC_ddest_signal.clear();
        dC_dkernel.clear();
    }
    real* k=kernel.data();
    real* dCdk=dC_dkernel.data();
    real* dCdd=dC_ddest_signal.data();
    real* d=dest_signal.data();
    real* dCds=dC_dsource_signal.data();

    //! for i=0 to nd-1:
    //!   for j=0 to nk-1:
    //!     dC_ddest_signal[i] += dC_dsource_signal[i*step+j]*kernel[j]
    //!     dC_dkernel[j] += dC_dsource_signal[i*step+j]*dest_signal[i]
    for (int i=0;i<nd;i++,dCds+=step)
    {
        real di = d[i];
        real dCdd_i_sum = 0;
        for (int j=0;j<nk;j++)
        {
            dCdd_i_sum += dCds[j]*k[j];
            dCdk[j] += dCds[j]*di;
        }
        dCdd[i] += dCdd_i_sum;
    }
}


//! Same as above, but increments only dC/dkernel, not  dC/ddest_signal
//!    dC/dkernel[j] += sum_{i=0}^{ND-1} dC_dsource_signal[i+j]*dest_signal[i]
void backConvolve1Dbackprop(const Vec& dest_signal,
                            const Vec& dC_dsource_signal,
                            const Vec& dC_dkernel,
                            int step, bool accumulate)
{
    int nk=dC_dkernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    int ns=dC_dsource_signal.length();
    if (step<1)
        PLERROR("backConvolve1Dbackprop: step (%d) should be a positive"
                " integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("backConvolve1Dbackprop: dC_dsource_signal.length() (%d)\n"
                "should equal %d:\n"
                "step (%d) * (dest_signal.length() (%d) - 1) + dC_dkernel.length()"
                " (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
#endif
    if (!accumulate)
        dC_dkernel.clear();

    real* dCdk=dC_dkernel.data();
    real* d=dest_signal.data();
    real* dCds=dC_dsource_signal.data();

    //! for i=0 to nd-1:
    //!   for j=0 to nk-1:
    //!     dC_dkernel[j] += dC_dsource_signal[i*step+j]*dest_signal[i]
    for (int i=0;i<nd;i++,dCds+=step)
    {
        real di = d[i];
        for (int j=0;j<nk;j++)
            dCdk[j] += dCds[j]*di;
    }
}


//! Convolve a (N1S x N2S) source image with a (N1K x N2K) kernel matrix,
//! and put result in a destination matrix of dimensions (N1D x N2D), stepping
//! by (step1,step2) in each direction, with NiS = NiD*stepi + NiK - 1.
//! The destination image is
//!    dest_image[i,j] = 
//!      sum_{k1=0}^{N1K-1} sum_{k2=0}^{N2K-1} source_image[i*step1+k1,j*step2+k2]*kernel[k1,k2]
void convolve2D(const Mat& source_image, const Mat& kernel,
                const Mat& dest_image,
                int step1, int step2, bool accumulate)
{
    int n1k=kernel.length();
    int n2k=kernel.width();
    int n1d=dest_image.length();
    int n2d=dest_image.width();
#ifdef BOUNDCHECK
    int n1s=source_image.length();
    int n2s=source_image.width();
    if (step1<1)
        PLERROR("convolve2D: step1 (%d) should be a positive integer\n",step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("convolve2D: source_image.length() (%d) should equal %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);

    if (step2<1)
        PLERROR("convolve2D: step2 (%d) should be a positive integer\n",step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("convolve2D: source_image.width() (%d) should equal %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) + kernel.width()"
                " (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
#endif
    if (!accumulate)
        dest_image.clear();
    int sm = source_image.mod();
    int dm = dest_image.mod();
    int km = kernel.mod();
    real* s = source_image.data();
    real* d = dest_image.data();
    for (int i=0;i<n1d;i++,s+=sm*step1,d+=dm)
    {
        real* s1 = s; // copy to iterate over columns
        for (int j=0;j<n2d;j++,s1+=step2)
        {
            real somme=0;
            real* k = kernel.data();
            real* ss = s1; // copy to iterate over sub-rows
            for (int l=0;l<n1k;l++,ss+=sm,k+=km)
                for (int m=0;m<n2k;m++)
                    somme += ss[m]*k[m];
            d[j]+=somme;
        }
    }
}

//! Back-convolve INTO a (N1S x N2S) "source" image with a (N1K x N2K)
//! kernel matrix, and FROM a "destination" image of dimensions (N1D x
//! N2D), with NiS = NiD + NiK - 1.
//! This is EXACTLY the TRANSPOSE of convolve2D(source_image, kernel,
//! dest_image, 1, 1) with the same arguments, computations flowing in
//! the other direction.
//! The kernel window is stepped by one in both directions. The
//! destination image is
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     source_image[i1+j1,i2+j2] += dest_image[i1,i2]*kernel[j1,j2]
//! If the accumulate flag is not set, then source_image is first cleared.
//! N.B. When dest_image has been computed from kernel and source_image
//! using convolve2D, THIS IS THE SAME AS COMPUTING dC/dsource_image
//! (into the source_image argument), GIVEN dC/ddest_image, i.e.
//! this function does part of the work done by convolve2Dbackprop.
void backConvolve2D(const Mat& source_image, const Mat& kernel,
                    const Mat& dest_image,
                    int step1, int step2, bool accumulate)
{
    int n1k=kernel.length();
    int n2k=kernel.width();
    int n1d=dest_image.length();
    int n2d=dest_image.width();
#ifdef BOUNDCHECK
    int n1s=source_image.length();
    int n2s=source_image.width();
    if (step1<1)
        PLERROR("backConvolve2D: step1 (%d) should be a positive integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("backConvolve2D: source_image.length() (%d) should equal %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);

    if (step2<1)
        PLERROR("backConvolve2D: step2 (%d) should be a positive integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("backConvolve2D: source_image.width() (%d) should equal %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) + kernel.width()"
                " (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
#endif
    if (!accumulate)
        source_image.clear();
    int sm = source_image.mod();
    int dm = dest_image.mod();
    int km = kernel.mod();
    real* s = source_image.data();
    real* d = dest_image.data();
    for (int i=0;i<n1d;i++,s+=sm*step1,d+=dm)
    {
        real* s1 = s; // copy to iterate over columns
        for (int j=0;j<n2d;j++,s1+=step2)
        {
            real* k = kernel.data();
            real* ss = s1; // copy to iterate over sub-rows
            real d_ij=d[j];
            for (int l=0;l<n1k;l++,ss+=sm,k+=km)
            {
                for (int m=0;m<n2k;m++)
                    ss[m] += d_ij * k[m];
            }
        }
    }
}

//! Increment dC/dsource_image and dC/dkernel, given dC/ddest_image, with
//! dest_image computed as per convolve2D(source_image, kernel, dest_image):
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     dC/dsource_image[i1+j1,i2+j2] += dC/dest_image[i1,i2]*kernel[j1,j2]
//!     dC/dkernel[j1,j2] += dC/dest_image[i1,i2]*source_image[i1+j1,i2+j2]
void convolve2Dbackprop(const Mat& source_image, const Mat& kernel,
                        const Mat& dC_ddest_image,
                        const Mat& dC_dsource_image, const Mat& dC_dkernel,
                        int step1, int step2, bool accumulate)
{
    int n1k=kernel.length();
    int n2k=kernel.width();
    int n1d=dC_ddest_image.length();
    int n2d=dC_ddest_image.width();
#ifdef BOUNDCHECK
    int n1s=source_image.length();
    int n2s=source_image.width();
    if (step1<1)
        PLERROR("convolve2Dbackprop: step1 (%d) should be a positive integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("convolve2Dbackprop: source_image.length() (%d) should equal"
                " %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);
    if (dC_dsource_image.length()!=n1s)
        PLERROR("convolve2Dbackprop: source_image.length() (%d) should"
                " equal:\n"
                "dC_dsource_image.length() (%d)\n",
                n1s,dC_dsource_image.length());
    if (dC_dkernel.length()!=n1k)
        PLERROR("convolve2Dbackprop: kernel.length() (%d) should equal:\n"
                " dC_dkernel.length() (%d)\n",
                n1k,dC_dkernel.length());

    if (step2<1)
        PLERROR("convolve2Dbackprop: step2 (%d) should be a positive integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("convolve2Dbackprop: source_image.width() (%d) should equal"
                " %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) + kernel.width()"
                " (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
    if (dC_dsource_image.width()!=n2s)
        PLERROR("convolve2Dbackprop: source_image.width() (%d) should"
                " equal:\n"
                "dC_dsource_image.width() (%d)\n",
                n2s,dC_dsource_image.width());
    if (dC_dkernel.length()!=n2k)
        PLERROR("convolve2Dbackprop: kernel.width() (%d) should equal:\n"
                " dC_dkernel.width() (%d)\n",
                n2k,dC_dkernel.width());
#endif
    if (!accumulate)
    {
        dC_dsource_image.clear();
        dC_dkernel.clear();
    }
    int sm = source_image.mod();
    int dCdsm = dC_dsource_image.mod();
    int km = kernel.mod();
    int dCdkm = dC_dkernel.mod();
    int dCddm = dC_ddest_image.mod();

    real* s = source_image.data();
    real* dCds = dC_dsource_image.data();
    real* dCdd = dC_ddest_image.data();

    for (int i=0;i<n1d;i++,s+=sm*step1,dCds+=dCdsm*step1,dCdd+=dCddm)
    {
        real* s1 = s; // copy to iterate over columns
        real* dCds1 = dCds;
        for (int j=0;j<n2d;j++,s1+=step2,dCds1+=step2)
        {
            real* k = kernel.data();
            real* dCdk = dC_dkernel.data();
            real* ss = s1; // copy to iterate over sub-rows
            real* dCdss = dCds1;
            real dCdd_ij=dCdd[j];
            for (int l=0;l<n1k;l++,ss+=sm,dCdss+=dCdsm,k+=km,dCdk+=dCdkm)
            {
                for (int m=0;m<n2k;m++)
                {
                    dCdss[m] += dCdd_ij * k[m];
                    dCdk[m] += dCdd_ij * ss[m];
                }
            }
        }
    }
}


//! As above, but increments only dC/dkernel, not dC/dsource_image
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     dC/dkernel[j1,j2] += dC/dest_image[i1,i2]*source_image[i1+j1,i2+j2]
void convolve2Dbackprop(const Mat& source_image,
                        const Mat& dC_ddest_image,
                        const Mat& dC_dkernel,
                        int step1, int step2, bool accumulate)
{
    int n1k=dC_dkernel.length();
    int n2k=dC_dkernel.width();
    int n1d=dC_ddest_image.length();
    int n2d=dC_ddest_image.width();
#ifdef BOUNDCHECK
    int n1s=source_image.length();
    int n2s=source_image.width();
    if (step1<1)
        PLERROR("convolve2Dbackprop: step1 (%d) should be a positive integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("convolve2Dbackprop: source_image.length() (%d) should equal"
                " %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) +"
                " dC_dkernel.length() (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);

    if (step2<1)
        PLERROR("convolve2Dbackprop: step2 (%d) should be a positive integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("convolve2Dbackprop: source_image.width() (%d) should equal"
                " %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) +"
                " dC_dkernel.width() (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
#endif
    if (!accumulate)
        dC_dkernel.clear();

    int sm = source_image.mod();
    int dCdkm = dC_dkernel.mod();
    int dCddm = dC_ddest_image.mod();

    real* s = source_image.data();
    real* dCdd = dC_ddest_image.data();

    for (int i=0;i<n1d;i++,s+=sm*step1,dCdd+=dCddm)
    {
        real* s1 = s; // copy to iterate over columns
        for (int j=0;j<n2d;j++,s1+=step2)
        {
            real* dCdk = dC_dkernel.data();
            real* ss = s1; // copy to iterate over sub-rows
            real dCdd_ij=dCdd[j];
            for (int l=0;l<n1k;l++,ss+=sm,dCdk+=dCdkm)
                for (int m=0;m<n2k;m++)
                    dCdk[m] += dCdd_ij * ss[m];
        }
    }
}


//! Increment dC/ddest_image and dC/dkernel, given dC/dsource_image, with
//! source_image computed as per
//! backConvolve2D(source_image, kernel, dest_image):
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     dC/ddest_image[i1,i2] += dC/dsource_image[i1+j1,i2+j2]*kernel[j1,j2]
//!     dC/dkernel[j1,j2] += dC/dsource_image[i1+j1,i2+j2]*dest_image[i1,i2]
void backConvolve2Dbackprop(const Mat& kernel, const Mat& dest_image,
                            const Mat& dC_ddest_image,
                            const Mat& dC_dsource_image, const Mat& dC_dkernel,
                            int step1, int step2, bool accumulate)
{
    int n1k=kernel.length();
    int n2k=kernel.width();
    int n1d=dest_image.length();
    int n2d=dest_image.width();
#ifdef BOUNDCHECK
    int n1s=dC_dsource_image.length();
    int n2s=dC_dsource_image.width();
    if (step1<1)
        PLERROR("backConvolve2Dbackprop: step1 (%d) should be a positive"
                " integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("backConvolve2Dbackprop: dC_dsource_image.length() (%d)\n"
                "should equal %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);
    if (dC_ddest_image.length()!=n1d)
        PLERROR("backConvolve2Dbackprop: dest_image.length() (%d) should"
                " equal:\n"
                "dC_ddest_image.length() (%d)\n",
                n1d,dC_ddest_image.length());
    if (dC_dkernel.length()!=n1k)
        PLERROR("backConvolve2Dbackprop: kernel.length() (%d) should equal:\n"
                " dC_dkernel.length() (%d)\n",
                n1k,dC_dkernel.length());

    if (step2<1)
        PLERROR("backConvolve2Dbackprop: step2 (%d) should be a positive"
                " integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("backConvolve2Dbackprop: source_image.width() (%d)\n"
                "should equal %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) + kernel.width()"
                " (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
    if (dC_ddest_image.width()!=n2d)
        PLERROR("backConvolve2Dbackprop: dest_image.width() (%d) should"
                " equal:\n"
                "dC_ddest_image.width() (%d)\n",
                n2d,dC_ddest_image.width());
    if (dC_dkernel.length()!=n2k)
        PLERROR("backConvolve2Dbackprop: kernel.width() (%d) should equal:\n"
                " dC_dkernel.width() (%d)\n",
                n2k,dC_dkernel.width());
#endif
    if (!accumulate)
    {
        dC_ddest_image.clear();
        dC_dkernel.clear();
    }
    int km = kernel.mod();
    int dCdkm = dC_dkernel.mod();
    int dm = dest_image.mod();
    int dCddm = dC_ddest_image.mod();
    int dCdsm = dC_dsource_image.mod();

    real* d = dest_image.data();
    real* dCdd = dC_ddest_image.data();
    real* dCds = dC_dsource_image.data();

    for (int i=0;i<n1d;i++,d+=dm,dCdd+=dCddm,dCds+=dCdsm*step1)
    {
        real* dCds1 = dCds; // copy to iterate over columns
        for (int j=0;j<n2d;j++,dCds1+=step2)
        {
            real* k = kernel.data();
            real* dCdk = dC_dkernel.data();
            real* dCdss = dCds1; // copy to iterate over sub-rows
            real d_ij=d[j];
            real dCdd_ij_sum = 0;
            for (int l=0;l<n1k;l++,dCdss+=dCdsm,k+=km,dCdk+=dCdkm)
            {
                for (int m=0;m<n2k;m++)
                {
                    dCdd_ij_sum += dCdss[m]*k[m];
                    dCdk[m] += dCdss[m]*d_ij;
                }
            }
            dCdd[j] += dCdd_ij_sum;
        }
    }
}

//! As above, but increments only dC/dkernel, not dC/ddest_image
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     dC/dkernel[j1,j2] += dC/dsource_image[i1+j1,i2+j2]*dest_image[i1,i2]
void backConvolve2Dbackprop(const Mat& dest_image,
                            const Mat& dC_dsource_image,
                            const Mat& dC_dkernel,
                            int step1, int step2, bool accumulate)
{
    int n1k=dC_dkernel.length();
    int n2k=dC_dkernel.width();
    int n1d=dest_image.length();
    int n2d=dest_image.width();
#ifdef BOUNDCHECK
    int n1s=dC_dsource_image.length();
    int n2s=dC_dsource_image.width();
    if (step1<1)
        PLERROR("backConvolve2Dbackprop: step1 (%d) should be a positive"
                " integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("backConvolve2Dbackprop: dC_dsource_image.length() (%d)\n"
                "should equal %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) +"
                " dC_dkernel.length() (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);

    if (step2<1)
        PLERROR("backConvolve2Dbackprop: step2 (%d) should be a positive"
                " integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("backConvolve2Dbackprop: source_image.width() (%d)\n"
                "should equal %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) +"
                " dC_dkernel.width() (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
#endif
    if (!accumulate)
        dC_dkernel.clear();

    int dCdkm = dC_dkernel.mod();
    int dm = dest_image.mod();
    int dCdsm = dC_dsource_image.mod();

    real* d = dest_image.data();
    real* dCds = dC_dsource_image.data();

    for (int i=0;i<n1d;i++,d+=dm,dCds+=dCdsm*step1)
    {
        real* dCds1 = dCds; // copy to iterate over columns
        for (int j=0;j<n2d;j++,dCds1+=step2)
        {
            real* dCdk = dC_dkernel.data();
            real* dCdss = dCds1; // copy to iterate over sub-rows
            real d_ij=d[j];
            for (int l=0;l<n1k;l++,dCdss+=dCdsm,dCdk+=dCdkm)
                for (int m=0;m<n2k;m++)
                    dCdk[m] += dCdss[m]*d_ij;
        }
    }
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
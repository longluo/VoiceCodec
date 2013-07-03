/*
  CELP voice codec, part of the HawkVoice Direct Interface (HVDI)
  cross platform network voice library
  Copyright (C) 2001-2003 Phil Frisbie, Jr. (phil@hawksoft.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
    
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA  02111-1307, USA.
      
  Or go to http://www.gnu.org/copyleft/lgpl.html
*/

#include "celpint.h"
#include "ftol.h"

#define N	128
#define NB	15
#define EPS	1.e-6

/* PI/2 */
#define MYPI_S2          1.57079632679489661923
/* 2/PI */
#define MYIPI_S2          0.63661977236758134308

double mycos(double f)
{
    double f2;
    double co;
    int i1;
    
    /* bring the argument f to the [-PI/2, PI/2] interval */
    i1=lrintf((float)(f*MYIPI_S2)); 
    if(i1&1) i1++;
    f-=i1*MYPI_S2;
    
    /* the real cos routine */
    f2=f*f;
    co=1.0+f2*(-0.4999999963 + f2*(0.0416666418 + f2*(-0.0013888397 + 
        f2*(0.0000247609 - f2*0.0000002605))));

    /* this handles the values with negative cosines */
    return ((i1&2)?-co:co);
}

#define cos(x) mycos(x)

void pctolsp2(float *a, int m, float *freq, celp_encoder_state *st)
{
    float p[MAXORD], q[MAXORD], ang, fm, tempfreq;
    float fr, pxr, tpxr, tfr, pxm, pxl, fl, qxl, tqxr;
    float qxm, qxr, tqxl;
    int mp, mh, nf, mb, jc, i, j;
    
    mp = m + 1;
    mh = m / 2;
    
    /* *generate p and q polynomials				 	*/
    
    for (i = 0; i < mh; i++)
    {
        p[i] = a[i+1] + a[m-i];
        q[i] = a[i+1] - a[m-i];
    }
    
    /* *compute p at f=0.							*/
    
    fl = 0.;
    for (pxl = 1.0, j = 0; j < mh; j++)
        pxl += p[j];
    
    /* *search for zeros of p						*/
    
    nf = 0;
    for (i = 1; i <= N; pxl = tpxr, fl = tfr, i++)
    {
        mb = 0;
        fr = i * (0.5f / N);
        pxr = (float)cos(mp * M_PI * fr);
        for (j = 0; j < mh; j++)
        {
            jc = mp - (j+1)*2;
            ang = jc * M_PI * fr;
            pxr += (float)cos(ang) * p[j];
        }
        tpxr = pxr;
        tfr = fr;
        if (pxl * pxr > 0.0) continue;
        
        do
        {
            mb++;
            fm = fl + (fr-fl) / (pxl-pxr) * pxl;
            pxm = (float)cos(mp * M_PI * fm);
            
            for (j = 0; j < mh; j++)
            {
                jc = mp - (j+1) * 2;
                ang = jc * M_PI * fm;
                pxm += (float)cos(ang) * p[j];
            }
            (pxm*pxl > 0.0) ? (pxl = pxm, fl = fm) : (pxr = pxm, fr = fm);
            
        } while ((fabs(pxm) > EPS) && (mb < 4));
        
        if ((pxl-pxr) * pxl == 0) 
        {
            for (j = 0; j < m; j++)
                freq[j] = (j+1) * 0.04545f;
            return;
        }
        freq[nf] = fl + (fr-fl) / (pxl-pxr) * pxl;
        nf += 2;
        if (nf > m-2) break;
    }
    
    
    /* *search for the zeros of q(z)					*/
    
    freq[m] = 0.5;
    fl = freq[0];
    qxl = (float)sin(mp * M_PI * fl);
    for (j = 0; j < mh; j++)
    {
        jc = mp - (j+1) * 2;
        ang = jc * M_PI * fl;
        qxl += (float)sin(ang) * q[j];
    }
    
    for (i = 2; i < mp; qxl = tqxr, fl = tfr, i += 2)
    {
        mb = 0;
        fr = freq[i];
        qxr = (float)sin(mp * M_PI * fr);
        for (j = 0; j < mh; j++)
        {
            jc = mp - (j+1) * 2;
            ang = jc * M_PI * fr;
            qxr += (float)sin(ang) * q[j];
        }
        tqxl = qxl;
        tfr = fr;
        tqxr = qxr;
        
        do
        {
            mb++;
            fm = (fl+fr) * 0.5f;
            qxm = (float)sin(mp * M_PI * fm);
            
            
            for (j = 0; j < mh; j++)
            {
                jc = mp - (j+1) * 2;
                ang = jc * M_PI * fm;
                qxm += (float)sin(ang) * q[j];
            }
            (qxm*qxl > 0.0) ? (qxl = qxm, fl = fm) : (qxr = qxm, fr = fm);
            
        } while ((fabs(qxm) > EPS*tqxl) && (mb < NB));
        
        if ((qxl-qxr) * qxl == 0)
        {
            for (j = 0; j < m; j++)
                freq[j] = st->lastfreq[j];
            return;
        }
        freq[i-1] = fl + (fr-fl) / (qxl-qxr) * qxl;
    }
    
    for (i = 1; i < m; i++)
    {
        
        /* *reorder lsps if non-monotonic					*/
        
        if (freq[i]  <  freq[i-1]) 
        {
            tempfreq = freq[i];
            freq[i] = freq[i-1];
            freq[i-1] = tempfreq;
        }
    }
    
    /* *if non-monotonic after 1st pass, reset to last values		*/
    
    for (i = 1; i < m; i++)
    {
        if (freq[i]  <  freq[i-1])
        {
            for (j = 0; j < m; j++)
                freq[j] = st->lastfreq[j];
            break;
        }
    }
    for (i = 0; i < m; i++) 
        st->lastfreq[i] = freq[i];
}

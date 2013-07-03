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

#include <memory.h>
#include "celpint.h"

#define LEN		30  /* *length of truncated impulse response	 */
#define MAXBUFPTR	MMAX + MAXNO + 2  * MAXLP + MAXNP - 1

static int submult[MAXPD][4] = 
{
#include "submult.h"      /* *load pitch submultiple delay table   	 */
    
};
extern float h[MAXLP], pdelay[MAXPD];

extern float pitchencode(float input, int *index);
extern float pgain(float *ex, int l, int first, int m, int len, float *match, celp_encoder_state *st);

static void movefr(int n, float *a, float *b)
{
    int i;
    
    for (i = 0; i < n; i++)
        *b++ = *a++;
}

void psearch(int l, int *pindex, celp_encoder_state *st, int *tauptr, int *minptr, float *d1b)
{
    int i, m, lag, start;
    int first, bigptr, subptr, topptr, maxptr, bufptr;
    float g[MAXPD], match[MAXPD], emax;
    
    /*	See warning below ----------------  \/ max (MAXL, MAXLP)	*/
    
    float v0[MAXBUFPTR], v0shift[MAXLP], frac;
    
    /* *initialize arrays							 */
    memset(v0, 0, sizeof(v0));
    memset(v0shift, 0, sizeof(v0shift));
    memset(g, 0, sizeof(g));
    memset(match, 0, sizeof(match));
    
    bufptr = MMAX + 10 + 2*l + MAXNP - 1;
    
    /* *update adaptive code book (pitch memory)				 */
    
    movefr(st->idb, d1b, &v0[bufptr - st->idb - l]);
    
    /* *initial conditions						 */
    
    if (st->nseg == 1)
    {
        st->bb[2] = 0.0;
        st->bb[0] = MMIN;
    }
    else
    {
        
        /*		*find allowable pointer range (minptr to maxptr)	 */
        
        if ((st->nseg % 2) == 0)
        {
            
            /* *delta delay coding on even subframes		 		 */
            
            *minptr = st->oldptr - (st->plevel2/2 - 1);
            maxptr = st->oldptr + (st->plevel2/2);
            if (*minptr < 0)
            {
                *minptr = 0;
                maxptr = st->plevel2 - 1;
            }
            if (maxptr > st->plevel1 - 1)
            {
                maxptr = st->plevel1 - 1;
                *minptr = st->plevel1 - st->plevel2;
            }
        }
        else
        {
            
            /* *full range coding on odd subframes				 */
            
            *minptr = 0;
            maxptr = st->plevel1 - 1;
        }
        
        start = bufptr - l + 1;
        
        
        /* *find gain and match score for integer pitch delays		 */
        /* *(using end-point correction on unity spaced delays)		 */
        
        first = TRUE;
        for (i = *minptr; i <= maxptr; i++)
        {
            m = (int) pdelay[i];
            frac = pdelay[i] - m;
            if (frac < 1.e-4f && frac > -1.e-4f)
            {
                lag = start - m;
                g[i] = pgain(&v0[lag-1], l, first, m, LEN, &match[i], st);
                first = FALSE;
            }
            else
                match[i] = 0.0f;
        }
        
        /* *find pointer to top (MSPE) match score (topptr)			 */
        /* *search for best match score (max -error term)			 */
        
        topptr = *minptr;
        emax = match[topptr];
        for (i = *minptr; i <= maxptr; i++)
        {
            if (match[i] > emax)
            {
                topptr = i;
                emax = match[topptr];
            }
        }
        
        /* *for full search (odd) subframes:				 */
        /* *select shortest delay of 2, 3, or 4 submultiples. if its match   */
        /* *is within 1 dB of MSPE to favor smooth "pitch"			 */
        
        *tauptr = topptr;
        if ((st->nseg % 2) != 0)
        {
            
            /* *for each submultiple {2, 3 & 4}				 */
            
            for (i = 1; i <= submult[topptr][0]; i++)
            {
                
                /* *find best neighborhood match for given submultiple	 */
                
                bigptr = submult[topptr][i];
                for (subptr = mmax(submult[topptr][i] - 8, *minptr); subptr <= 
                    mmin(submult[topptr][i] + 8, maxptr); subptr++)
                {
                    if (match[subptr] > match[bigptr])
                        bigptr = subptr;
                }
                
                /* *select submultiple match if within 1 dB MSPE match	 */
                
                if (match[bigptr] >= 0.88f * match[topptr])
                {
                    *tauptr = bigptr;
                }
            }
        }
        
        /* *place pitch parameters in common bb "structure"			 */
        
        st->bb[2] = g[*tauptr];
        st->bb[0] = pdelay[*tauptr];
        
        /* *save pitch pointer to determine delta delay			 */
        
        st->oldptr = *tauptr;
        
  }
  
  /* *pitch quantization bb[2]						 */
  
  st->bb[2] = pitchencode(st->bb[2], pindex);
}


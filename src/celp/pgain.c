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

/* NOTE: Phil Frisbie broke up pgain into two routines to get around a MSVC 5 bug */
void pgain2(float *ex, int l, int first, int m, int len, celp_encoder_state *st,
            float *y2, float *c, float *e)
{
    float cor = *c, eng = *e;
    int i, j;
    
    if(st->fastgain == 1)
    {
        if (first)
        {
            for (i = 0; i < l; i++)
            {
                st->yp[i] = 0.0f;
            }
            for (i = 0; i < len; i++)
            {
                for (j = 0; j <= i; j++)
                    st->yp[i] += st->h[j] * ex[i - j];
            }
            for (i = len; i < l; i++)
            {
                st->yp[i] += st->h[0] * ex[i];
            }
        }
        else
        {
            for (i = len - 1; i > 0; i--)
                st->yp[i - 1] += ex[0] * st->h[i]; 
            
            for (i = l - 1; i > 0; i--)
                st->yp[i] = st->yp[i - 1];
            
            st->yp[0] = ex[0] * st->h[0];
        }
        
        /* *For lags (m) shorter than frame size (l), replicate the short
        *adaptive codeword to the full codeword length by
        *overlapping and adding the convolutions:			 	 */
        
        /* round m to an even number */
        m = (m / 2) * 2;
        if (m < l)			
        {
            for (i = 0; i < m; i+=2)
                y2[i] = st->yp[i];
            
            /* add in 2nd convolution		 				 */
            
            for (i = m; i < l; i+=2)
                y2[i] = st->yp[i] + st->yp[i - m];
            
            if (m < (l / 2))		
            {
                
                /* add in 3rd convolution		 				 */
                
                for (i = 2 * m; i < l; i+=2)
                    y2[i] += st->yp[i - 2 * m];
            }
        }
        else
        {
            for (i = 0; i < l; i+=2)
                y2[i] = st->yp[i];
        }
        /* *Calculate correlation and energy:
        e0 = r[n]   = spectrum prediction residual
        y2 = r[n-m] = error weighting filtered reconstructed
        pitch prediction signal (m = correlation lag)	 */
        
        cor = 0.0f;
        eng = 0.0f;
        
        for (i = 0; i < l; i+=2)
        {
            float temp = y2[i];
            
            cor += temp * st->e0[i];
            eng += temp * temp;
        }
        cor *= 2.0f;
        eng *= 2.0f;
    }
    else
    {
        if (first)
        {
            for (i = 0; i < l; i++)
            {
                st->yp[i] = 0.0f;
            }
            for (i = 0; i < len; i++)
            {
                for (j = 0; j <= i; j++)
                    st->yp[i] += st->h[j] * ex[i - j];
            }
            for (i = len; i < l; i++)
            {
                st->yp[i] += st->h[0] * ex[i];
            }
        }
        else
        {
            for (i = len - 1; i > 0; i--)
                st->yp[i - 1] += ex[0] * st->h[i]; 
            
            for (i = l - 1; i > 0; i--)
                st->yp[i] = st->yp[i - 1];
            
            st->yp[0] = ex[0] * st->h[0];
        }
        /* *For lags (m) shorter than frame size (l), replicate the short
        *adaptive codeword to the full codeword length by
        *overlapping and adding the convolutions:			 	 */
        
        if (m < l)			
        {
            for (i = 0; i < m; i++)
                y2[i] = st->yp[i];
            
            /* add in 2nd convolution		 				 */
            
            for (i = m; i < l; i++)
                y2[i] = st->yp[i] + st->yp[i - m];
            
            if (m < (l / 2))		
            {
                
                /* add in 3rd convolution		 				 */
                
                for (i = 2 * m; i < l; i++)
                    y2[i] += st->yp[i - 2 * m];
            }
        }
        else
        {
            for (i = 0; i < l; i++)
                y2[i] = st->yp[i];
        }
        /* *Calculate correlation and energy:
        e0 = r[n]   = spectrum prediction residual
        y2 = r[n-m] = error weighting filtered reconstructed
        pitch prediction signal (m = correlation lag)	 */
        
        cor = 0.0f;
        eng = 0.0f;
        
        for (i = 0; i < l; i++)
        {
            float temp = y2[i];
            
            cor += temp * st->e0[i];
            eng += temp * temp;
        }
    }
    *c = cor; *e = eng;
}

float pgain(float *ex, int l, int first, int m, int len, float *match, celp_encoder_state *st)
{
    float cor, eng;
    float y2[MAXLP], pg;
    
    pgain2(ex, l, first,m, len, st, y2, &cor, &eng);
    
    /* *Compute gain and error:
    NOTE: Actual MSPE = e0.e0 - pgain(2*cor-pgain*eng)
    since e0.e0 is independent of the code word,
    minimizing MSPE is equivalent to maximizing:
    match = pgain(2*cor-pgain*eng)   (1)
    If unquantized pgain is used, this simplifies:
    match = cor*pgain
    
      NOTE: Inferior results were obtained when quantized
      pgain was used in equation (1)???
      
        NOTE: When the delay is less than the frame length, "match"
        is only an approximation to the actual error.		
        
    Independent (open-loop) quantization of gain and match (index):	 */
    
    if (eng <= 0.0f) 
        eng = 1.0f;
    
    pg = cor / eng;
    *match = cor * pg;
    
    return (pg);
}

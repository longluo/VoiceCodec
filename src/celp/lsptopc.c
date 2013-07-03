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

void lsptopc(float *f, float *pc)
{
    int i, j, k, lspflag;
    float freq[MAXNO], p[MAXNO / 2], q[MAXNO / 2];
    float a[MAXNO / 2 + 1], a1[MAXNO / 2 + 1], a2[MAXNO / 2 + 1];
    float b[MAXNO / 2 + 1], b1[MAXNO / 2 + 1], b2[MAXNO / 2 + 1];
    float xx, xf;
    
    /* *check input for ill-conditioned cases 			 */
    
    lspflag = FALSE;
    for (i = 1; i < 10; i++)
    {
        if (f[i] <= f[i - 1])
            lspflag = TRUE;
    }
    /* *initialization 						 */
    
    for (j = 0; j < 10; j++)
        freq[j] = f[j];

    memset(a, 0, sizeof(a[0]) * (MAXNO / 2 + 1));
    memset(a1, 0, sizeof(a[0]) * (MAXNO / 2 + 1));
    memset(a2, 0, sizeof(a[0]) * (MAXNO / 2 + 1));
    memset(b, 0, sizeof(a[0]) * (MAXNO / 2 + 1));
    memset(b1, 0, sizeof(a[0]) * (MAXNO / 2 + 1));
    memset(b2, 0, sizeof(a[0]) * (MAXNO / 2 + 1));

    /* *lsp filter parameters 					 */
    
    for (i = 0; i < (MAXNO / 2); i++)
    {
        p[i] = -2.f * (float)cos(2.f * M_PI * freq[2 * i]);
        q[i] = -2.f * (float)cos(2.f * M_PI * freq[2 * i + 1]);
    }
    
    /* *impulse response of analysis filter 			 */
    
    xf = 0.0f;
    for (k = 0; k < 10 + 1; k++)
    {
        xx = 0.0;
        if (k == 0)
            xx = 1.0f;
        a[0] = xx + xf;
        b[0] = xx - xf;
        xf = xx;
        for (i = 0; i < (MAXNO / 2); i++)
        {
            a[i + 1] = a[i] + p[i] * a1[i] + a2[i];
            b[i + 1] = b[i] + q[i] * b1[i] + b2[i];
            a2[i] = a1[i];
            a1[i] = a[i];
            b2[i] = b1[i];
            b1[i] = b[i];
        }
        if (k != 0)
            pc[k - 1] = -.5f * (a[MAXNO / 2] + b[MAXNO / 2]);
    }
    
    /* *convert to CELP's predictor coefficient array configuration */
    
    for (i = 10 - 1; i >= 0; i--)
        pc[i + 1] = -pc[i];
    pc[0] = 1.0f;
}

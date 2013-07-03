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

static void bwexp1(float alpha, float *pc, float *pcexp, int n)
{
    int i;
    float temp = 1.0f;

    for (i = 0; i <= n; i++)
    {
        pcexp[i] = pc[i] * temp;
        temp *= alpha;
    }
}

static void cor(float *rar, int idim, float *c0, float *c)
{
    int i, k;
    
    for (*c0 = 0.0, i = 0; i < idim; i++)
        *c0 += rar[i] * rar[i];

    memset(c, 0, sizeof(c[0]) * MAXNO);
    for (i = 0; i < MAXNO; i++)
    {
        for (k = i+1; k < idim; k++)
            c[i] += rar[k] * rar[k-i-1];
    }
}

static void durbin(float c0, float *c, float *a)
{
    int i, j;
    float alpha, beta, rc[MAXNO], tmp[MAXNO];
    
    /* If zero energy, set rc's to zero & return  */
    
    if (c0 <= 0.0f) 
    {
        memset(rc, 0, sizeof(rc[0]) * MAXNO);
        return;
    }
    
    /* Intialization   */
    
    alpha = c0;
    *a = rc[0] = -*c / c0;
    beta = *c;
    
    /* Recursion   */
    
    for (i = 1; i < MAXNO; i++)
    {
        alpha += beta * rc[i - 1];
        beta = c[i];
        
        for (j = 0; j <= i - 1; j++)
            beta +=  c[j] * a[i-j-1];
        rc[i] = -beta / alpha;
        
        for (j = 0; j <= i - 1; j++)
            tmp[j] = rc[i] * a[i-j-1];
        
        for (j = 0; j <= i - 1; j++)
            a[j] += tmp[j];
        a[i] = rc[i];
    }
}       

void autohf(float *si, float *w, int n, float omega, float *a)
{
    int i;
    float c0, c[MAXNO], atemp[MAXNO+1], s[MAXLL];

    memset(atemp, 0, sizeof(atemp));
    
    for (i = 0;  i < n ; i++)
        s[i] = si[i] * w[i];		/* apply window			*/

    cor(s, n, &c0, c);			/* calculate autocorrelations	*/

    atemp[0] = 1.0f;			/* convert autocorrelations to pc's  */
    durbin(c0, c, &atemp[1]);

    bwexp1(omega, atemp, a, MAXNO);		/* expand corrected pc's	*/
}

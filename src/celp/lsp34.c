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

#define FSCALE	8000.0f
#define MIN_LSF_DIST	39

static int lsp[MAXNO][16] =
{
    { 100,  170,  225,  250,  280,  340,  420,  500},
    { 210,  235,  265,  295,  325,  360,  400,  440,
    480,  520,  560,  610,  670,  740,  810,  880},
    { 420,  460,  500,  540,  585,  640,  705,  775,
    850,  950, 1050, 1150, 1250, 1350, 1450, 1550},
    { 620,  660,  720,  795,  880,  970, 1080, 1170,
    1270, 1370, 1470, 1570, 1670, 1770, 1870, 1970},
    {1000, 1050, 1130, 1210, 1285, 1350, 1430, 1510,
    1590, 1670, 1750, 1850, 1950, 2050, 2150, 2250},
    {1470, 1570, 1690, 1830, 2000, 2200, 2400, 2600},
    {1800, 1880, 1960, 2100, 2300, 2480, 2700, 2900},
    {2225, 2400, 2525, 2650, 2800, 2950, 3150, 3350},
    {2760, 2880, 3000, 3100, 3200, 3310, 3430, 3550},
    {3190, 3270, 3350, 3420, 3490, 3590, 3710, 3830},
};

void lsp34(float *freq, int no, int *bits, int *findex)
{
    int levels, i, j;
    float dist, low, errorup, errordn;
    
    /* *sequentially find closest quantized LSP indicies */
    
    for (i = 0; i < no; i++)
    {
        freq[i] *= FSCALE;
        levels = (1 << bits[i]) - 1;
        
        /* *Quantize to nearest output level			 */
        
        low = dist = (float)fabs(freq[i] - *lsp[i]);
        findex[i] = 0;
        for (j = 1; j <= levels; j++)
        {
            dist = (float)fabs(freq[i] - lsp[i][j]);
            if (dist < low)
            {
                low = dist;
                findex[i] = j;
            }
        }
        
        /* *adjust quantization if nonmonotonically quantized
        *find minimum quantization error adjustment 		 */
        
        
        if (i > 0)
        {
            if (lsp[i][findex[i]] - MIN_LSF_DIST <= lsp[i - 1][findex[i - 1]])
            {
                errorup = (float)fabs(freq[i] - lsp[i][mmin(findex[i] + 1, levels)]) +
                    (float)fabs(freq[i - 1] - lsp[i - 1][findex[i - 1]]);
                errordn = (float)fabs(freq[i] - lsp[i][findex[i]]) +
                    (float)fabs(freq[i - 1] - lsp[i - 1][mmax(findex[i - 1] - 1, 0)]);
                
                /* *adjust index for minimum error (and preserve monotonicity!) */
                
                if (errorup < errordn)
                {
                    findex[i] = mmin(findex[i] + 1, levels);
                    while (lsp[i][findex[i]] - MIN_LSF_DIST < lsp[i-1][findex[i-1]])
                        findex[i] = mmin(findex[i] + 1, levels);
                }
                else if (i == 1)
                    findex[i - 1] = mmax(findex[i - 1] - 1, 0);
                else if (lsp[i - 1][mmax(findex[i - 1] - 1, 0)]  - MIN_LSF_DIST >
                    lsp[i - 2][findex[i - 2]])
                    findex[i - 1] = mmax(findex[i - 1] - 1, 0);
                else
                {
                    findex[i] = mmin(findex[i] + 1, levels);
                    while (lsp[i][findex[i]]  - MIN_LSF_DIST < lsp[i-1][findex[i-1]])
                        findex[i] = mmin(findex[i] + 1, levels);
                }
            }
        }
    }
    
    /* *quantize lsp frequencies using indicies found above */
    
    for (i = 0; i < no; i++)
        freq[i] = lsp[i][findex[i]] / FSCALE;
}
/**************************************************************************
*
* ROUTINE
*               lspdecode34
*
* FUNCTION
*                
*               independent nonuniform scalar lsp decoder 
*
* SYNOPSIS
*               subroutine lspdecode34(findex,no,freq)
*
*   formal 
*
*                       data    I/O
*       name            type    type    function
*       -------------------------------------------------------------------
*       findex		int	i	lsp frequency index
*	no		int	i	lpc order
*	freq		float	o	lsp quantized frequency
*
***************************************************************************
*
* DESCRIPTION
*	George Kang's tables modified for no preemphasis and bit allocation
*
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*	
*
**************************************************************************/

void lspdecode34(int *findex, int no, float *freq)
{
    int i;
    
    /* *** choose appropriate frequency by findex */
    
    for (i = 0; i < no; i++)
    {
        freq[i] = lsp[i][findex[i]]/FSCALE;
    }
}

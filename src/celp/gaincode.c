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

/* *Log quantization                               */

static float gainlog5[32] =
{
    -1330., -870., -660., -520., -418., -340., -278., -224.,
        -178., -136.,  -98.,  -64.,  -35.,  -13.,   -3.,   -1.,
        1.,    3.,   13.,   35.,   64.,   98.,  136.,  178.,
        224.,  278.,  340.,  418.,  520.,  660.,  870., 1330.
};

float gainencode(float input, int *index)
{
    int i;
    static float midpoints[31] = 
    {
        -1100., -765., -590., -469., -379., -309., -251., -201.,
            -157., -117.,  -81.,  -49.5, -24.,   -8.,   -2.,    0.,
            2.,    8.,   24.,   49.5,  81.,  117.,  157.,  201.,
            251.,  309.,  379.,  469.,  590.,  765., 1100.
    };
    
    /* *Binary tree search for closest gain				 */
    
    for (*index = 15, i = 8; i >= 1; i = i >> 1)
    {
        if (input > midpoints[*index])
            *index += i;
        else
            *index -= i;
    }
    if (input > midpoints[*index])
        (*index)++;
    
    /* *Return quantized gain and ZERO based index			 */
    
    return (gainlog5[*index]);
}

float gainencode2(float numer, float denom, int *index)
{
    
    /* *Hard coded for 5 bit quantization to achieve high speed  	 	*/
    
    int i;
    static float midpoints[31] = 
    {
        -1100., -765., -590., -469., -379., -309., -251., -201.,
            -157., -117.,  -81.,  -49.5, -24.,   -8.,   -2.,    0.,
            2.,    8.,   24.,   49.5,  81.,  117.,  157.,  201.,
            251.,  309.,  379.,  469.,  590.,  765., 1100.
    };
    
    /* *Binary tree search for closest gain				 */
    
    for (*index = 15, i = 8; i >= 1; i = i >> 1)
    {
        if (numer  > denom * midpoints[*index])
            *index += i;
        else
            *index -= i;
    }
    if (numer > denom * midpoints[*index])
        (*index)++;
    
    /* *Return quantized gain and ZERO based index			 */
    
    return (gainlog5[*index]);
}

void gaindecode(int gindex, int bits, float *gain)
{
    /* Choose appropriate gain                                         */
    
    if (bits == 5)
        *gain = gainlog5[gindex];
}

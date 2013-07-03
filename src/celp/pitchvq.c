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

void pitchvql(float *rar, int idim, float *buf, int idimb, float *b)
{
    int k, m, i, start;
    
    k = idimb - idim;
    start = k + 1;
    m = (int)b[0];
    
    /* *update memory							 */
    
    for (i = 0; i < k; i++)
        buf[i] = buf[i + idim];
    
    /* *update memory with selected pitch memory from selected delay (m)    */
    
    for (i = k; i < idimb; i++)	
        buf[i] = buf[i - m];
    
    /* *return "rar" with scaled memory added to stochastic contribution	 */
    
    for (i = 0; i < idim; i++)	
        buf[i + k] = rar[i] += b[2] * buf[i + k]; 
    
}


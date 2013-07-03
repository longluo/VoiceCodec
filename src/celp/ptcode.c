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

static float pitch2max5[32] =
{
    -0.993f, -0.831f, -0.693f, -0.555f, -0.414f, -0.229f,    0.0f,  0.139f,
        0.255f,  0.368f,  0.457f,  0.531f,  0.601f,  0.653f,  0.702f,  0.745f,
        0.780f,  0.816f,  0.850f,  0.881f,  0.915f,  0.948f,  0.983f,  1.020f, 
        1.062f,  1.117f,  1.193f,  1.289f,  1.394f,  1.540f,  1.765f,  1.991f
};

float pitchencode(float input, int *index)
{
    int i;
    float dist, low;
    
    low = dist = (float)fabs(input - *pitch2max5);
    *index = 0;
    for (i = 1; i < 32; i++)
    {
        dist = (float)fabs(input - pitch2max5[i]);
        if (dist < low)
        {
            low = dist;
            *index = i;
        }
    }
    return (pitch2max5[*index]);
}

void pitchdecode(int pindex, float *pitch)
{
    *pitch = pitch2max5[pindex];
}

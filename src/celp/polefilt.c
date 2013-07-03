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


void polefilt10(float *a, float *z, float *xy, int len)
{
    int t;
    float z0;

    for (t = 0; t < len; t++)
    {
        z0 = xy[t];
        z0 -= a[10] * z[10];
        z[10]  = z[9];
        z0 -= a[9] * z[9];
        z[9]  = z[8];
        z0 -= a[8] * z[8];
        z[8]  = z[7];
        z0 -= a[7] * z[7];
        z[7]  = z[6];
        z0 -= a[6] * z[6];
        z[6]  = z[5];
        z0 -= a[5] * z[5];
        z[5]  = z[4];
        z0 -= a[4] * z[4];
        z[4]  = z[3];
        z0 -= a[3] * z[3];
        z[3]  = z[2];
        z0 -= a[2] * z[2];
        z[2]  = z[1];
        z0 -= a[1] * z[1];
        z[1]  = z0;
        xy[t] = z0;
    }
}

void polefilt3(float *a, float *z, float *xy, int len)
{
    int t;
    float z0;
    
    for (t = 0; t < len; t++)
    {
        z0 = xy[t];
        
        z0 -= a[3] * z[3];
        z[3]  = z[2];
        
        z0 -= a[2] * z[2];
        z[2]  = z[1];
        
        z0 -= a[1] * z[1];
        z[1]  = z0;
        
        xy[t] = z0;
    }
}

void polefilt2(float *a, float *z, float *xy, int len)
{
    int t;
    float z0;
    
    for (t = 0; t < len; t++)
    {
        z0 = xy[t];
        
        z0 -= a[2] * z[2];
        z[2]  = z[1];
        
        z0 -= a[1] * z[1];
        z[1]  = z0;
        
        xy[t] = z0;
    }
}


void polefilt(float *a, int n, float *z, float *xy, int len)
{
    int t, j;
    
    for (t = 0; t < len; t++)
    {
        z[0] = xy[t];
        for (j = n; j > 0; j--)
        {
            z[0] -= a[j] * z[j];
            z[j]  = z[j-1];
        }
        xy[t] = z[0];
    }
}

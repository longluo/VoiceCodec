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

void zerofilt10(float *b, float *z, float *xy, int len)
{
    register float ar;
    int t;
    
    for (t = 0; t < len; t++)
    {
        z[0] = xy[t];
        ar   = 0.0;

        ar  += z[10] * b[10];
        z[10] = z[9];

        ar  += z[9] * b[9];
        z[9] = z[8];

        ar  += z[8] * b[8];
        z[8] = z[7];

        ar  += z[7] * b[7];
        z[7] = z[6];

        ar  += z[6] * b[6];
        z[6] = z[5];

        ar  += z[5] * b[5];
        z[5] = z[4];

        ar  += z[4] * b[4];
        z[4] = z[3];

        ar  += z[3] * b[3];
        z[3] = z[2];

        ar  += z[2] * b[2];
        z[2] = z[1];

        ar  += z[1] * b[1];
        z[1] = z[0];

        xy[t] = ar + z[0] * b[0];
    }
}

void zerofilt2(float *b, float *z, float *xy, int len)
{
    register float ar;
    int t;
    
    for (t = 0; t < len; t++)
    {
        z[0] = xy[t];
        ar   = 0.0;

        ar  += z[2] * b[2];
        z[2] = z[1];

        ar  += z[1] * b[1];
        z[1] = z[0];

        xy[t] = ar + z[0] * b[0];
    }
}

void zerofilt(float *b, int n, float *z, float *xy, int len)
{
    register float ar;
    int t, j;
    
    for (t = 0; t < len; t++)
    {
        z[0] = xy[t];
        ar   = 0.0;
        for (j = n; j > 0; j--)
        {
            ar  += z[j] * b[j];
            z[j] = z[j-1];
        }
        xy[t] = ar + z[0] * b[0];
    }
}

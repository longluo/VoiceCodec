/*
  LPC voice codec, part of the HawkVoice Direct Interface (HVDI)
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

#ifdef _MSC_VER
#pragma warning (disable:4056) /* to disable bogus warning in MSVC 5.0 */
#pragma warning (disable:4001) /* to disable warning in <math.h> */
#pragma warning (disable:4711) /* to disable automatic inline warning */
#endif

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "lpc.h"
#include "ftol.h"

#define LPC_FILTORDER			10
#define BUFLEN			((LPC_SAMPLES_PER_FRAME * 3) / 2)

typedef struct lpc_d_state{
		float Oldper, OldG, Oldk[LPC_FILTORDER], bp[LPC_FILTORDER + 1];
		int pitchctr;
        float exc;
} lpc_d_state_t;


lpc_decoder_state *create_lpc_decoder_state(void)
{
    lpc_decoder_state *state;
    
    state = (lpc_decoder_state *)malloc(sizeof(lpc_decoder_state));
    
    return state;
}

void init_lpc_decoder_state(lpc_decoder_state *st)
{
    int i;
    
    st->Oldper = 0.0f;
    st->OldG = 0.0f;
    for (i = 0; i <= LPC_FILTORDER; i++) {
        st->Oldk[i] = 0.0f;
        st->bp[i] = 0.0f;
    }
    st->pitchctr = 0;
    st->exc = 0.0f;
}

/* PN random number generator by Phil Frisbie */
/* returns all numbers from 1 to 65536 */
__inline int PNrandom (void)
{
    static int the_random = 2385;
    
    if((the_random & 1) == 1)
    {
        the_random = ((the_random >> 1) ^ 0xfff6) | 0x8000;
    }
    else
    { 
        the_random >>= 1;
    }
    
    return(the_random);
}

int lpc_decode(unsigned char *in, short *out, lpc_decoder_state *st)
{
    int i;
    register float u, f, per, G, NewG, Ginc, Newper, perinc;
    float k[LPC_FILTORDER], Newk[LPC_FILTORDER];
    float bp0, bp1, bp2, bp3, bp4, bp5, bp6, bp7, bp8, bp9, bp10;
    float b, kj;
    
    bp0 = st->bp[0];
    bp1 = st->bp[1];
    bp2 = st->bp[2];
    bp3 = st->bp[3];
    bp4 = st->bp[4];
    bp5 = st->bp[5];
    bp6 = st->bp[6];
    bp7 = st->bp[7];
    bp8 = st->bp[8];
    bp9 = st->bp[9];
    bp10 = st->bp[10];
    
    per = (float)(in[0]);
    per = (float)(per / 2);
    
    G = (float)in[1] / 256.f;
    k[0] = 0.0;
    for (i = 0; i < LPC_FILTORDER; i++)
        k[i] = (float) ((signed char)in[i + 2]) * 0.0078125f;
    
    G /= (float)sqrt(BUFLEN / ((per < FLT_EPSILON && per > -FLT_EPSILON)? 3.0f : per));
    Newper = st->Oldper;
    NewG = st->OldG;

    for (i = 0; i < LPC_FILTORDER; i++)
        Newk[i] = st->Oldk[i];
    
    if (st->Oldper != 0 && per != 0) {
        perinc = (per - st->Oldper) * (1.0f/LPC_SAMPLES_PER_FRAME);
        Ginc = (G - st->OldG) * (1.0f/LPC_SAMPLES_PER_FRAME);
    } else {
        perinc = 0.0f;
        Ginc = 0.0f;
    }
    
    if (Newper == 0.f)
        st->pitchctr = 0;
    
    for (i = 0; i < LPC_SAMPLES_PER_FRAME; i++) {
        if (Newper == 0.f) {
            u = (((PNrandom() - 32768) / 32768.0f)) * 1.5874f * NewG; 
        } else {
            if (st->pitchctr == 0) {
                u = NewG;
                st->pitchctr = (int) Newper;
            } else {
                u = 0.0f;
                st->pitchctr--;
            }
        }
        
        f = u;
        
        b = bp9;
        kj = Newk[9];
        f -= kj * bp9;
        bp10 = bp9 + kj * f;
        
        kj = Newk[8];
        f -= kj * bp8;
        bp9 = bp8 + kj * f;
        
        kj = Newk[7];
        f -= kj * bp7;
        bp8 = bp7 + kj * f;
        
        kj = Newk[6];
        f -= kj * bp6;
        bp7 = bp6 + kj * f;
        
        kj = Newk[5];
        f -= kj * bp5;
        bp6 = bp5 + kj * f;
        
        kj = Newk[4];
        f -= kj * bp4;
        bp5 = bp4 + kj * f;
        
        kj = Newk[3];
        f -= kj * bp3;
        bp4 = bp3 + kj * f;
        
        kj = Newk[2];
        f -= kj * bp2;
        bp3 = bp2 + kj * f;
        
        kj = Newk[1];
        f -= kj * bp1;
        bp2 = bp1 + kj * f;
        
        kj = Newk[0];
        f -= kj * bp0;
        bp1 = bp0 + kj * f;
        
        bp0 = f;
        
        u = f;
        if (u  < -0.9999f) {
            u = -0.9999f;
        } else if (u > 0.9999f) {
            u = 0.9999f;
        }
        *out++ = (short)lrintf(u * 32767.0f);
        Newper += perinc;
        NewG += Ginc;
    }
    st->bp[0] = bp0;
    st->bp[1] = bp1;
    st->bp[2] = bp2;
    st->bp[3] = bp3;
    st->bp[4] = bp4;
    st->bp[5] = bp5;
    st->bp[6] = bp6;
    st->bp[7] = bp7;
    st->bp[8] = bp8;
    st->bp[9] = bp9;
    st->bp[10] = bp10;
    
    st->Oldper = per;
    st->OldG = G;
    for (i = 0; i < LPC_FILTORDER; i++)
        st->Oldk[i] = k[i];
    
    return LPC_SAMPLES_PER_FRAME;
}

void destroy_lpc_decoder_state(lpc_decoder_state *st)
{
    if(st != NULL)
    {
        free(st);
        st = NULL;
    }
}


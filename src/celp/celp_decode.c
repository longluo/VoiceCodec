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
/**************************************************************************
*                                                                         *
*	CELP Voice Coder                                                      *
*	Version 3.2c	                                                      *
*									                                      *
*	The U.S. Government shall not be held liable for any damages          *
*	resulting from this code.  Further reproduction or distribution       *
*	of this code without prior written permission of the U.S.             *
*	Government is prohibited.  	                                          *
*                                                                         *
***************************************************************************
*
* ROUTINE
*		celp_decode
*
* FUNCTION
*		Codebook excited linear predictor (main routine)
*
***************************************************************************/

#ifdef _MSC_VER
#pragma warning (disable:4305) /* to disable const double to float warning */
#endif

#include <stdlib.h>
#ifndef MACOSX
#include <malloc.h>
#endif
#include <memory.h>
#include "celpint.h"
#include "ftol.h"

extern void zerofilt10(float *b, float *z, float *xy, int len);

extern void zerofilt2(float *b, float *z, float *xy, int len);
extern void polefilt10(float *a, float *z, float *xy, int len);
extern void polefilt2(float *a, float *z, float *xy, int len);
extern void ham(float *win, int n);
extern void lspdecode34(int *findex, int no, float *freq);
extern void lsptopc(float *f, float *pc);
extern void pitchvql(float *rar, int idim, float *buf, int idimb, float *b);
extern void gaindecode(int gindex, int bits, float *gain);
extern void pitchdecode(int pindex, float *pitch);

/* defined in celp_encode.c */
extern int pbits[MAXNP + 2];
extern float pdelay[MAXPD];
extern int sbits[MAXNO];
extern float w[2][4];
extern float bhpf[3];
extern float ahpf[3];

/***************** Decoding routines start here ****************/
#define TC          0.01f
#define ALPHA		0.8f  /* Bandwidth expansion for postfilter */
#define BETA		0.5f  /* Bandwidth expansion for postfilter */

/* *load stochastic code book vector file			*/
float x[MAXCODE] = 
{
#include "codebook.h"
};

static void bwexp(float alpha, float *pc, float *pcexp, int n)
{
    int i;
    float temp = 1.0f;

    for (i = 0; i <= n; i++)
    {
        pcexp[i] = pc[i] * temp;
        temp *= alpha;
    }
}

static void unpack(int *array, int bits, int *value, int *pointer)
{
    int i;
    
    for (i = 0, *value = 0; i < bits; i++, (*pointer)++)
        *value |= array[*pointer+1] << i;
}

static void pctorc(float *lpc, float *rc)
{
    float t[MAXNO+1], a[MAXNO+1];
    int i, j;
    
    for (i = 0; i <= 10; i++)
        a[i] = lpc[i];
    for (i = 10; i > 1; i--)
    {
        rc[i-1] = -a[i];
        for (j = 1; j < i; j++)
            t[i-j] = (a[i-j] + rc[i-1] * a[j]) / (1.0f - rc[i-1] * rc[i-1]);
        for (j = 1; j < i; j++)
            a[j] = t[j];
    }
    rc[0] = -a[1];
}

static int clip(float *s, int l)
{
    int i, count;
    float sum;
    
    /*	Count number of clippings and sum their magnitudes		*/
    
    count = 0;
    sum = 0.0;
    for (i = 0; i < l; i++)
    {
        if (fabs(s[i]) > 32768.0f)
        {
            count++;
            sum += (float)fabs(s[i]);
        }
    }
    
    /*	Clipping heuristics (could also use energy, delta energy, etc.)	*/
    
    return(((count >= 10) || (count >= 5 && sum > 1.e6)) ? TRUE : FALSE);
}

static void dcodcbg(int cbgbits, int bitsum1, int bitsum2, int *bitpointer,
                    int *stream, float *cbg)
{
    int i, pointer, index;
    
    pointer = *bitpointer;
    for (i = 0; i < NSUBFRAMES; i++)
    {
        unpack(stream, cbgbits, &index, &pointer);
        gaindecode(index, cbgbits, &cbg[i]);
        if (i == 0 || i == 2 || i == 4)
            pointer += bitsum2 - cbgbits;
        else
            pointer += bitsum1 - cbgbits;
    }
    *bitpointer += cbgbits;
}

static void dcodcbi(int cbbits, int bitsum1, int bitsum2, int *bitpointer,
                    int *stream, int *cbi)
{
    int i, pointer;
    
    pointer = *bitpointer;
    for (i = 0; i < NSUBFRAMES; i++)
    {
        unpack(stream, cbbits, &cbi[i], &pointer);
        cbi[i]++;
        if (i == 0 || i == 2 || i == 4)
            pointer += bitsum2 - cbbits;
        else
            pointer += bitsum1 - cbbits;
    }
    *bitpointer += cbbits;
}

static void dcodpg(int pgbits, int bitsum1, int bitsum2, int *bitpointer,
                   int *stream, float *pgs)
{
    int i, pointer, index;
    
    pointer = *bitpointer;
    for (i = 0; i < NSUBFRAMES; i++)
    {
        unpack(stream, pgbits, &index, &pointer);
        pitchdecode(index, &pgs[i]);
        if (i == 0 || i == 2 || i == 4)
            pointer += bitsum2 - pgbits;
        else
            pointer += bitsum1 - pgbits;
    }
    *bitpointer += pgbits;
}

static void dcodtau(int taubits, int taudelta, int bitsum1, int bitsum2, int *bitpointer,
                    int *stream, float *taus, celp_decoder_state *st)
{
    int i, pointer, tptr, mxptr, mnptr;
    static int lptr = 0;
    
    pointer = *bitpointer;
    for (i = 0; i < NSUBFRAMES; i++)
    {
        if (((i + 1) % 2) != 0)
        {
            unpack(stream, taubits, &tptr, &pointer);
            taus[i] = pdelay[tptr];
            pointer += bitsum1 - taubits;
        }
        else
        {
            unpack(stream, taudelta, &tptr, &pointer);
            pointer += bitsum2 - taudelta;
            mnptr = lptr - (st->plevel2 / 2 - 1);
            mxptr = lptr + (st->plevel2 / 2);
            if (mnptr < 0)
                mnptr = 0;
            if (mxptr > st->plevel1 - 1)
                mnptr = st->plevel1 - st->plevel2;
            taus[i] = pdelay[tptr + mnptr];
        }
        lptr = tptr;
    }
    *bitpointer += taubits;
}

static void intsynth(float *lspnew, float lsp[][MAXNO], celp_decoder_state *st)
{
    int i, j;
    
    /* *interpolate lsp's						 */
    for (i = 0; i < NSUBFRAMES; i++)
    {
        for (j = 0; j < 10; j++)
            lsp[i][j] = w[0][i] * st->lspold[j] + w[1][i] * lspnew[j];
    }
    
    /* 		*update lsp history					 */
    
    for (i = 0; i < 10; i++)
        st->lspold[i] = lspnew[i];
}

static void zerofilt1(float *b, float *z, float *xy, int len)
{
    register float ar;
    int t;
    
    for (t = 0; t < len; t++)
    {
        z[0] = xy[t];
        ar   = 0.0;

        ar  += z[1] * b[1];
        z[1] = z[0];

        xy[t] = ar + z[0] * b[0];
    }
}

static void postfilt(float *s, int l, float *fci, celp_decoder_state *st)
{
    int n;
    float ast[2];
    float pcexp1[MAXNO + 1], pcexp2[MAXNO + 1], rcexp2[MAXNO];
    float powerin = st->ip;
    float powerout = st->op;
    float *dp1 = st->dp1, *dp2 = st->dp2, *dp3 = st->dp3;
    
#ifdef POSTFIL2
    float scale;
#else
    float newpowerin[MAXL + 1], newpowerout[MAXL + 1];
#endif
    
    /*			*estimate input power				*/
    
#ifdef POSTFIL2
    for (n = 0; n < l; n++)
        powerin = powerin * (1.0f - TC) + TC * s[n] * s[n];
#else
    newpowerin[0] = powerin;
    for (n = 0; n < l; n++)
        newpowerin[n + 1] = (1.0f - TC) * newpowerin[n] + TC * s[n] * s[n];
    powerin = newpowerin[l];
#endif
    
    /* *BW expansion							*/
    bwexp(BETA, fci, pcexp1, 10);
    bwexp(ALPHA, fci, pcexp2, 10);
    
    /* *pole-zero postfilter						*/
    zerofilt10(pcexp1, dp1, s, l);
    polefilt10(pcexp2, dp2, s, l);
    
    /* *find spectral tilt (1st order fit) of postfilter
    *(denominator dominates the tilt)					*/
    pctorc(pcexp2, rcexp2);
    
    /* *tilt compensation by a scaled zero
    *(don't allow hF roll-off)						*/
    ast[0] = 1.0;
    ast[1] = (rcexp2[0] > 0.f) ? -0.5f * rcexp2[0] : 0.f;
    zerofilt1(ast, dp3, s, l);
    
    /* *estimate output power						*/
    
#ifdef POSTFIL2
    for (n = 0; n < l; n++)
        powerout = powerout * (1.0f - TC) + TC * s[n] * s[n];
    
    /* *block wise automatic gain control					*/
    
    if (powerout > 0.0)
        for (scale = (float)sqrt(powerin / powerout), n = 0; n < l; n++)
            s[n] *= scale;
#else
        newpowerout[0] = powerout;
        for (n = 0; n < l; n++)
            newpowerout[n + 1] = (1.0f - TC) * newpowerout[n] + TC * s[n] * s[n];
        powerout = newpowerout[l];
        
        /* *sample wise automatic gain control				*/
        
        for (n = 0; n < l; n++)
        {
            if (newpowerout[n + 1] > 0.0f)
                s[n] *= (float)sqrt(newpowerin[n + 1] / newpowerout[n + 1]);
        }
#endif
        st->ip = powerin;
        st->op = powerout;
}

static void vdecode(float decodedgain, int l, float *vdecoded, int cbindex)
{
    int i, codeword;
    
    /* *copy selected vector to excitation array	 		 	 */
    
    codeword = 2 * (MAXNCSIZE - cbindex);
    if (codeword < 0)
    {
        codeword = 0;
    }
    for (i = 0; i < l; i++)
        vdecoded[i] = x[i + codeword] * decodedgain;
}

int celp_decode(const unsigned char *in, short *out, celp_decoder_state *st)
{
    int cbindex;
    float bbd[MAXNP + 1];
    float fci[MAXNO + 1];
    int cbi[MAXLL / MAXL];
    int i, j, k, l = 60;
    int findex[MAXNO];
    float cbg[MAXLL / MAXL], pgs[MAXLL / MAXL];
    float vdecoded[MAXLL];
    float decodedgain, taus[4];
    float newfreq[MAXNO], lsp[MAXLL / MAXL][MAXNO];
    int pointer, bitpointer;
    int stream[STREAMBITS];
    
    /* *number of codewords/LPC frame				 */
    
    l = st->framesize / NSUBFRAMES;
    
    /* unpack bits into array */
    for (i = 0; i < STREAMBITS; i++)
    {
        stream[i] = (int)((in[i >> 3] & (1 << (i & 7))) != 0 ? 1 : 0);
    }
    
    /* *** SYNTHESIS ..........................................		 */
    
    /* *unpack data stream						 */
    pointer = -1; 
    
    /* stream is the encoded bits in */
    for (i = 0; i < 10; i++)
        unpack(stream, sbits[i], &findex[i], &pointer);
    
    /* *decode lsp's							 */
    
    lspdecode34(findex, 10, newfreq);
    
    /* *interpolate spectrum lsp's for subframes			 */
    
    intsynth(newfreq, lsp, st);
    
    
    /* *decode all code book and pitch parameters			 */
    
    bitpointer = pointer;
    dcodtau(pbits[0], pbits[1], st->bitsum1, st->bitsum2, &bitpointer, stream, taus, st);
    dcodpg(pbits[2], st->bitsum1, st->bitsum2, &bitpointer, stream, pgs);
    dcodcbi(pbits[3], st->bitsum1, st->bitsum2, &bitpointer, stream, cbi);
    dcodcbg(pbits[4], st->bitsum1, st->bitsum2, &bitpointer, stream, cbg);
    
    /* *** synthesize each subframe					 */
    
    k = 0;
    for (i = 0; i < NSUBFRAMES; i++)
    {
        /* *decode values for subframe					 */
        
        cbindex = cbi[i];
        decodedgain = cbg[i];
        
        /* *code book synthesis						 */
        
        vdecode(decodedgain, l, &vdecoded[k], cbindex);
        
        bbd[0] = taus[i];
        bbd[2] = pgs[i];
        
        /* *pitch synthesis						 */
        
        pitchvql(&vdecoded[k], l, st->dps, st->idb, bbd);
        
        /* convert lsp's to pc's 						 */
        
        lsptopc(&lsp[i][0], fci);	
        
        /* lpc synthesis	 						 */ 
        
        polefilt10(fci, st->dss, &vdecoded[k], l);	
        
        /* *** post filtering						 */
        
        postfilt(&vdecoded[k], l, fci, st);
        
        /* *** test for output speech clipping				 */
        
        while (clip(&vdecoded[k], l))
        {
            for (j = 0; j < l; j++)
                vdecoded[k + j] = 0.05f * vdecoded[k + j];
        }
#ifdef HIGHPASS_OUT
        /* *high pass filter						 */

        zerofilt2(ahpf, st->dhpf1, &vdecoded[k], l);
        polefilt2(bhpf, st->dhpf2, &vdecoded[k], l);
#endif
        /* *** write postfiltered output  */
        
        for (j = 0; j < l; j++)
            *out++ = (short)mmax(-32768, mmin(32767, lrintf(vdecoded[k + j])));
        
        k += l;
    }
    
    return st->framesize;
}

celp_decoder_state *create_celp_decoder_state()
{
    celp_decoder_state *st;
    
    st = (celp_decoder_state *)calloc(1, sizeof(celp_decoder_state));

    return st;
}

void init_celp_decoder_state(celp_decoder_state *st, int framesize)
{
    float lspold[MAXNO] = {.03f, .05f, .09f, .13f, .19f, .23f, .29f, .33f, .39f, .44f};
    int i;
    
    if(framesize > MAXFRAME || framesize < CELP_4_5_FRAMESIZE)
    {
        st->framesize = CELP_4_5_FRAMESIZE;
    }
    else
    {
        st->framesize = framesize;
    }
    st->framesize = framesize;
    st->plevel1 = 1 << pbits[0];
    st->plevel2 = 1 << pbits[1];
    for(i=0;i<MAXNO;i++)
        st->lspold[i] = lspold[i];
    st->idb =  MMAX + MAXNP + framesize/NSUBFRAMES - 1;
    /* *number of bits per subframe				 */
    st->bitsum1 =  pbits[0] + pbits[2] + pbits[3] + pbits[4];
    st->bitsum2 =  pbits[1] + pbits[2] + pbits[3] + pbits[4];
}

void destroy_celp_decoder_state(celp_decoder_state *st)
{
    free(st);
}


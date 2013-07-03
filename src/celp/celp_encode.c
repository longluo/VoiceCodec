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
*		celp_encode
*
* FUNCTION
*		Codebook excited linear predictor (main routine)
*
***************************************************************************
*
* REFERENCES
*
C	National Communication System Technical Information Bulletin
C	Federal Standard 1016 (to be published 1992).
C
C	Campbell, Joseph P. Jr., Thomas E. Tremain and Vanoy C. Welch,
C	"The Federal Standard 1016 4800 bps CELP Voice Coder," Digital
C	Signal Processing, Academic Press, Vol1, No. 3, p. 145-155.
C
C	Kemp, David, P., Retha A. Sueda and Thomas E. Tremain, "An
C	Evaluation of 4800 bps Voice Coders," Proceedings of the IEEE
C	International Conference on Acoustics, Speech and Signal Processing
C	(ICASSP), 1989, p. 200-203.
C
C	Fenichel, R., "Federal Standard 1016," National Communications
C	System, Office of Technology and Standards, Washington, DC 20305-2010,
C	14 February 1991.
C
C	Campbell, Joseph P. Jr., Thomas E. Tremain and Vanoy C. Welch,
C	"The DoD 4.8 kbps Standard (Proposed Federal Standard 1016),"
C	"Advances in Speech Coding", Kluwer Academic Publishers, 1991,
C	Chapter 12, p. 121-133.
C
C   Tutorials:
C	Fallside, Frank and William Woods, Computer Speech Processing,
C	Prentice Hall International, 1985, Chapter 4 (by Bishnu Atal).
*
***************************************************************************
*
*			4800 bps CELP Characteristics
*
*                  Spectrum       Pitch            Code Book
*                  -------------  ---------------  -----------------
*       Update     30 ms          30/4 = 7.5 ms    30/4 = 7.5 ms
*                  ll=240         lp=60            l=60
*
*       Order      10             256 (max) x 60   512 (max) x 60
*				   1 gain	      1 gain
*
*       Analysis   Open loop      Closed loop      Closed loop
*                  Correlation    Modified MSPE    MSPE VQ
*                  30 ms Hamming  VQ, weight=0.8   weight=0.8
*                  no preemphasis range 20 to 147  shift by 2
*                  15 Hz BW exp   (w/ fractions)   77% sparsity
*
*       Bits per   34 indep LSP   index:  8+5+8+5  index:     8*4
*        Frame     [3444433333]   gain(-1,2): 5*4  gain(+/-): 5*4
*
*       Bit Rate   1133.3 bps     1533.33 bps      1733.33 bps
*
***************************************************************************
*
*		UNPERMUTED BIT ASSIGNMENT
*
*	lsp  1	1-3		    lsp  6	20-22
*	lsp  2	4-7		    lsp  7	23-25
*	lsp  3	8-11		lsp  8	26-28
*	lsp  4	12-15		lsp  9	29-31
*	lsp  5	16-19		lsp 10	32-34
*
*	Subframe:	  1	      2	      3	        4
*	----------	-----	-----	-----	 -------
*   pitch delay	35-42	.....	84-91	 .......  8 bits
*   delta delay	.....	61-65	.......	 110-114  5 bits
*   pitch gain 	43-47	66-70	92-96	 115-119  5 bits
*   cbindex		48-55	71-78	97-104	 120-127  8 bits
*   cbgain		56-60	79-83	105-109	 128-132  5 bits
*               -----   -----   -------  -------
*   total/frame   26      23      26       23
**************************************************************************
* 
*   global, most are in the state structure
*
*
*	SPECTRUM VARIABLES:
*	fcn	float	new weighting filter coefficients
*	fci	float	interpolated weighting filter coefficients
*	gamma2	float	weight factor
*	no	int	filter order  predictor
*
*	PITCH VARIABLES:
*	bb	float	pitch predictor coefficients
*	idb	int	dimension of d1a and d1b???
*	tauptr  int	pitch delay pointer
*	minptr	int	minimum delay pointer
*	pbits	int	pitch gain coding bit allocation
*	pindex	int	pitch gain index bb[2]
*	pdelay	float	pitch delay coding table
*	plevel1	float	number of full search pitch delays
*	plevel2	float	number of delta search pitch delays
*
*	CODE BOOK VARIABLES:
*	cbindex	int	code book index
*	gindex	int	gain index
*	h	float	impulse response
*	ncsize	int	code book size
*	nseg	int	segment counter
*	x	float	code book
*
*
*	
**************************************************************************/
#ifdef _MSC_VER
#pragma warning (disable:4305) /* to disable const double to float warning */
#endif

#include <stdlib.h>
#ifndef MACOSX
#include <malloc.h>
#endif
#include <memory.h>
#include "celpint.h"

extern void pctolsp3(float *a, float *freq, int *sbits, int *findex);
extern void pctolsp2(float *a, int m, float *freq, celp_encoder_state *st);
extern void zerofilt2(float *b, float *z, float *xy, int len);
extern void polefilt2(float *a, float *z, float *xy, int len);
extern void autohf(float *si, float *w, int n, float omega, float *a);
extern void ham(float *win, int n);
extern void lsp34(float *freq, int no, int *bits, int *findex);
extern void lsptopc(float *f, float *pc);
extern void csub(float *s, float *v, int l, int *cbindex, int *gindex, celp_encoder_state *st,
                 float *fci, int *pindex, int *tauptr, int *minptr);

int pbits[MAXNP + 2] = {8, 5, 5, 8, 5};

/* *read adaptive code book index (pitch delay) file		*/
float pdelay[MAXPD] = 
{
#include "pdelay.h"
};

int sbits[MAXNO] = {3, 4, 4, 4, 4, 3, 3, 3, 3, 3};

float w[2][4] =   { {0.875f, 0.625f, 0.375f, 0.125f}, {0.125f, 0.375f, 0.625f, 0.875f}};

/***************** Encoding routines start here ****************/
#define OMEGA		0.994127f  /* Bandwidth expansion for LPC analysis (15 Hz) */

float bhpf[3] = {1.0f, -1.889033f, 0.8948743f};
float ahpf[3] = {0.946f, -1.892f, 0.946f};

static void intanaly(float *lspnew, float lsp[][MAXNO], celp_encoder_state *st)
{
    int i, j;
    
    for (i = 0; i < NSUBFRAMES; i++)
        
        /* *interpolate lsp's						 */
        
    {
        for (j = 0; j < 10; j++)
            lsp[i][j] = w[0][i]*st->lspold[j] + w[1][i]*lspnew[j];
        
    }
    
    /*		*save lsp's for next pass				*/
    
    for (j = 0; j < 10; j++)
    {
        st->lspold[j] = lspnew[j];
        st->oldlsp[j] = lsp[NSUBFRAMES][j];
    }
}

static void pack(int value, int bits, char *array, int *pointer)
{
    int i;
    
    for (i = 0; i < bits; (*pointer)++, i++)
        array[*pointer] = (char)((value & 1 << i) >> i);
}

int celp_encode(const short *in, unsigned char *out, celp_encoder_state *st)
{
    int cbindex = 0, gindex = 0, pindex = 0;
    int tauptr = 0, minptr = 0;
    int i, k, l;
    int findex[MAXNO];
    float fci[MAXNO + 1], fcn[MAXNO + 1];
    float snew[MAXLL], ssub[MAXLL], v[MAXLL];
    float newfreq[MAXNO + 1], lsp[MAXLL / MAXL][MAXNO];
    int pointer;
    char stream[STREAMBITS];
    
    l = st->framesize / NSUBFRAMES;
    
    /* *intialize arrays						*/
    
    for (i = 0; i < STREAMBITS; i++) stream[i] = 0;
    
    pointer = 0;
    
    for (i = 0; i < st->framesize; i++)
        snew[i] = (float)in[i];
    
    /* *high pass filter snew						 */

    zerofilt2(ahpf, st->dhpf1, snew, st->framesize);
    polefilt2(bhpf, st->dhpf2, snew, st->framesize);

    /* *make ssub vector from snew and sold			 	 */
    
    for (i = 0,k=st->framesize/2; i < st->framesize/2; i++,k++)
    {
        ssub[i] = st->sold[k];
        ssub[k] = snew[i];
    }
    
    autohf(snew, st->hamw, st->framesize, OMEGA, fcn);
    
    /* *pc -> lsp (new)							 */
#ifdef PCTOLSP3    
    pctolsp3(fcn, newfreq, sbits, findex);
#else
    pctolsp2(fcn, 10, newfreq, st);
    lsp34(newfreq, 10, sbits, findex);
#endif
    /* *pack lsp indices in bit stream array				 */
    
    for (i = 0; i < 10; i++)
        pack(findex[i], sbits[i], stream, &pointer);
    
    /* *linearly interpolate LSP's for each subframe			 */
    
    intanaly(newfreq, lsp, st);
    
    /* *** for each subframe, search stochastic & adaptive code books    */
    
    k = 0;
    for (i = 0; i < NSUBFRAMES; i++)
    {
        lsptopc(&lsp[i][0], fci);
        st->nseg++;
        
        /* *** code book & pitch searches					 */
        
        csub(&ssub[k], &v[k], l, &cbindex, &gindex, st, fci, &pindex, &tauptr, &minptr);
        
        /* *pitch quantization tau					 */
        
        /* *pack parameter indices in bit stream array			 */
        
        if ((st->nseg % 2) != 0)
            pack(tauptr, pbits[0], stream, &pointer);
        else
            pack(tauptr-minptr, pbits[1], stream, &pointer);
        
        pack(pindex, pbits[2], stream, &pointer);
        cbindex--;
        pack(cbindex, pbits[3], stream, &pointer);
        pack(gindex, pbits[4], stream, &pointer);
        k += l;
    }
    
    for (i = 0; i < st->framesize; i++)
        st->sold[i] = snew[i];
    
    /* pack the bits */
    memset(out, 0, STREAMBITS/8);
    for (i = 0; i < STREAMBITS; i++)
    {
        out[i >> 3] |= ((stream[i] != 0)? 1 : 0) << (i & 7);
    }
    
    return STREAMBITS/8;
}

celp_encoder_state *create_celp_encoder_state()
{
    celp_encoder_state *st;
    
    st = (celp_encoder_state *)calloc(1, sizeof(celp_encoder_state));

    return st;
}

void init_celp_encoder_state(celp_encoder_state *st, int framesize)
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
    st->plevel1 = 1 << pbits[0];
    st->plevel2 = 1 << pbits[1];
    st->idb =  MMAX + MAXNP + framesize/NSUBFRAMES - 1;
    st->cblength = MAXNCSIZE;
    for(i=0;i<MAXNO;i++)
        st->lspold[i] = lspold[i];
    /* *generate Hamming windows					 */
    ham(st->hamw, st->framesize);
    
    st->oldptr = 1;
    st->nseg = 0;
    st->gamma2 = 0.8f;
    st->fastgain = 0;
    st->inpulselen = MAX_IMPULSE;
}
void destroy_celp_encoder_state(celp_encoder_state *st)
{
    free(st);
}

void celp_set_encoder_option(celp_encoder_state *st, int option, int value)
{
    switch (option) {
    case CELP_FAST_GAIN:
        {
            if(value == 0)
            {
                st->fastgain = 0;
            }
            else
            {
                st->fastgain = 1;
            }
        }
        break;

    case CELP_CODEBOOK_LEN:
        st->cblength = mmin(mmax(value, 0), MAXNCSIZE);
        break;

    default:
        break;
    }
}

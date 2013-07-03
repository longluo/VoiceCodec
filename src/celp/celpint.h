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

#ifndef CELPINT_H
#define CELPINT_H

#ifdef _MSC_VER
#pragma warning (disable:4056) /* to disable bogus warning in MSVC 5.0 */
#pragma warning (disable:4001) /* to disable warning in <math.h> */
#pragma warning (disable:4711) /* to disable automatic inline warning */
#endif
#include <math.h>
#include "celp.h"

/* configure compile time options here */
/* define PCTOLSP3 to use the faster encoding routines */
#define PCTOLSP3
/* define POSTFIL2 to use the faster post-filtering routine */
#define POSTFIL2
/* define HIGHPASS_OUT to highpass filter the decoded speech */
//#define HIGHPASS_OUT


/*	CELP ARRAY SIZE PARAMETERS					 
	MAXNCSIZE 	maximum code book size	 (number of codewords)
	MAXL      	maximum codeword vector length
	MAXCODE   	maximum code book array length in samples
	MAXLL     	maximum LPC analysis frame size
	MAXNO     	maximum LPC filter order
	MAXLP     	maximum pitch prediction frame size	 
	MAXNP     	maximum pitch prediction order	 
	MMIN      	minimum delay pitch predictor (minimum value M)
	MMAX       	maximum delay pitch predictor (maximum value M)
	MAXPD		maximum number of pitch delays	 
	MAXPA      	maximum pitch analysis buffer length (use to be "idb")
			(assumes code book is updated faster or same as pitch)
	MAXM2		maximum array size for delay parameter 


*/
#define NSUBFRAMES  4

#define MAXFRAME  480
#define	MAXNCSIZE 256
#define	MAXL      (MAXFRAME/NSUBFRAMES)
#define	MAXCODE   2*(MAXNCSIZE)+MAXL /* shift by 2 overlapped code book */
#define	MAXLL     MAXFRAME
#define	MAXNO     10 
#define	MAXLP     (MAXFRAME/NSUBFRAMES) /* bug, fixed - See MAXPA */
#define	MAXNP     3
#define	MMIN      20
#define	MMAX      147
#define MAXPD	  256
#define MAXM2	  20
#define	MAXPA     MAXLP+MMAX+2+MAXM2

#define MAX_IMPULSE 30
#define MIN_IMPULSE 10

#define STREAMBITS	136
#define NFRAC 5
#define TRUE 1
#define FALSE 0
#define M1 -4
#define M2  3
#define MAXORD	24

#define mmax(A,B)      ((A)>(B)?(A):(B))
#define mmin(A,B)       ((A)<(B)?(A):(B))

#ifndef M_PI
	#define M_PI		3.14159265358979323846f
#endif

struct celp_enc_state {
    int idb, framesize;
    float lspold[MAXNO]; /* intanaly, intsynth */
    int plevel1, plevel2;
    float dps[MAXPA];
    float dhpf1[3], dhpf2[3]; /* celp_encode */

    /* encoding only */
    int cblength; /* the number of codebook entries to search, MAX 256 */
    int fastgain; /* if !0 then use fast gain routines */
    int inpulselen; /* length of truncated impulse response, max 30, min 10 */
    int oldptr;
    int nseg;
    float gamma2;
    float lastfreq[MAXORD];
    float hamw[MAXLL];
    float d1a[MAXPA], d1b[MAXPA];
    float d2a[MAXNO+1], d2b[MAXNO+1];
    float d3a[MAXNO+1], d3b[MAXNO+1];
    float d4a[MAXNO+1], d4b[MAXNO+1];
    float bb[MAXNP + 1];
    float h[MAXLP];
    float y[MAXL], y59save, y60save, eng;/* cgain */
    float ccor, e1, e0save[60]; /* mexcite */
    float yp[MAXLP]; /* pgain */
    float sold[MAXLL];
    float e0[MAXLP];
    float oldlsp[MAXNO]; /* intanaly */
};

struct celp_dec_state {
    int idb, framesize;
    float lspold[MAXNO]; /* intanaly, intsynth */
    int plevel1, plevel2;
    float dps[MAXPA];
    float dhpf1[3], dhpf2[3]; /* celp_decode */

    /* decoding only */
    float dss[MAXNO+1], dp1[MAXNO+1], dp2[MAXNO+1], dp3[2];
    float ip, op;
    int bitsum1, bitsum2;
};

#endif /* CELPINT_H */



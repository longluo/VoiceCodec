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

#include <memory.h>
#include "celpint.h"
#include "ftol.h"

extern float x[MAXCODE];

extern void psearch(int l, int *pindex, celp_encoder_state *st, int *tauptr, int *minptr, float *d1b);
extern void pitchvql(float *rar, int idim, float *buf, int idimb, float *b);
extern void polefilt10(float *a, float *z, float *xy, int len);
extern void polefilt3(float *a, float *z, float *xy, int len);
extern void zerofilt10(float *b, float *z, float *xy, int len);
extern float gainencode(float input, int *index);

static void bwexp10(float alpha, float *pc, float *pcexp)
{
    float temp = 1.0f;

    pcexp[0] = pc[0];
    temp *= alpha;

    pcexp[1] = pc[1] * temp;
    temp *= alpha;

    pcexp[2] = pc[2] * temp;
    temp *= alpha;

    pcexp[3] = pc[3] * temp;
    temp *= alpha;

    pcexp[4] = pc[4] * temp;
    temp *= alpha;

    pcexp[5] = pc[5] * temp;
    temp *= alpha;

    pcexp[6] = pc[6] * temp;
    temp *= alpha;

    pcexp[7] = pc[7] * temp;
    temp *= alpha;

    pcexp[8] = pc[8] * temp;
    temp *= alpha;

    pcexp[9] = pc[9] * temp;
}

static float fastcgain(float *ex, int l, int first, int len, float *match, celp_encoder_state *st)
{
    register float cor;
    float cgain;
    int i, j, minlen;
    float *y = st->y;
    float *h = st->h;
    float *pex, *py;
    float eng = st->eng;
    
    if (first)
    {
        pex = ex;
        memset(y, 0, sizeof(y[0]) * l);
        for (i = 0; i < l; i++,pex++)
        {
            if (*pex > 0.0f)
            {
                py = &y[i];
                minlen = mmin(l-i, len);
                for(j = 0; j < minlen; j++)
                    py[j] += h[j];
            }
            else if (*pex < 0.0f)
            {
                py = &y[i];
                minlen = mmin(l-i, len);
                for(j = 0; j < minlen; j++)
                    py[j] -= h[j];
            }
        }
    }
    else
    {
        /* *First and second shift combined			 */
        for (i = l - 2; i > len + 1; i-=2)
        {
            y[i] = y[i-2];
        }
        y[len] = y[len - 2] + ex[1] * h[len - 1];
        /* *ternary stochastic code book (-1, 0, +1)			 */
        if (ex[1] > 0.0f)
        {
            if (ex[0] > 0.0f)
            {
                for (i = len - 2; i > 1; i-=2)
                    y[i] = y[i-2] + h[i-1] + h[i];
            }
            else if(ex[0] < 0.0f)
            {
                for (i = len - 2; i > 1; i-=2)
                    y[i] = y[i-2] + h[i-1] - h[i];
            }
            else
            {
                for (i = len - 2; i > 1; i-=2)
                    y[i] = y[i-2] + h[i-1];
            }
        }
        else if (ex[1] < 0.0f)
        {
            if (ex[0] > 0.0f)
            {
                for (i = len - 2; i > 1; i-=2)
                    y[i] = y[i-2] - h[i-1] + h[i];
            }
            else if(ex[0] < 0.0f)
            {
                for (i = len - 2; i > 1; i-=2)
                    y[i] = y[i-2] - h[i-1] - h[i];
            }
            else
            {
                for (i = len - 2; i > 1; i-=2)
                    y[i] = y[i-2] - h[i-1];
            }
        }
        else
        {
            if (ex[0] > 0.0f)
            {
                for (i = len - 2; i > 1; i-=2)
                    y[i] = y[i-2] + h[i];
            }
            else if(ex[0] < 0.0f)
            {
                for (i = len - 2; i > 1; i-=2)
                    y[i] = y[i-2] - h[i];
            }
            else
            {
                for (i = len - 2; i > 1; i-=2)
                    y[i] = y[i-2];
            }
        }
        y[0] = ex[0] * h[0];
    }
    
    /**	Calculate correlation and energy:
    e0 = spectrum & pitch prediction residual
    y  = error weighting filtered code words
    
      \/\/\/  CELP's computations are focused in this correlation \/\/\/
      - For a 512 code book this correlation takes 4 MIPS!
    - Decimation?, Down-sample & decimate?, FEC codes?	*/
    
    cor = 0.0f;
    
    /* *End correct energy on subsequent code words:			 */
    if (lrintf(ex[0]) == 0 && lrintf(ex[1]) == 0 && !first)
    {
        py = y;
        for (i = 0; i < l; i+=2)
        {
            cor += y[i] * st->e0[i];
        }
        cor *= 2.0f;
        eng = eng - 2 * st->y59save * st->y59save;
    }
    else
    {
        py = y;
        eng = 0.0f;
        for (i = 0; i < l; i+=2)
        {
            float temp = py[i];

            eng += temp * temp;
            cor += temp * st->e0[i];
        }
        cor *= 2.0f;
        eng *= 2.0f;
    }
    st->y59save = y[l - 2];
    st->eng = eng;
    
    /*	Independent (open-loop) quantization of gain and match (index):	 */
    
    if (st->eng <= 0.0f)
        st->eng = 1.0f;
    cgain = cor / st->eng;
    *match = cor * cgain;
    
    return (cgain);
}

static float cgain(float *ex, int l, int first, int len, float *match, celp_encoder_state *st)
{
    register float cor;
    float cgain;
    int i, j, minlen;
    float *y = st->y;
    float *h = st->h;
    float *pex, *py;
    float eng = st->eng;
    
    if (first)
    {
        pex = ex;
        memset(y, 0, sizeof(y[0]) * l);
        for (i = 0; i < l; i++,pex++)
        {
            if (*pex > 0.0f)
            {
                py = &y[i];
                minlen = mmin(l-i, len);
                for(j = 0; j < minlen; j++)
                    py[j] += h[j];
            }
            else if (*pex < 0.0f)
            {
                py = &y[i];
                minlen = mmin(l-i, len);
                for(j = 0; j < minlen; j++)
                    py[j] -= h[j];
            }
        }
    }
    else
    {
        /* *First and second shift combined			 */

        for (i = l - 1; i > len + 1; i--)
        {
            y[i] = y[i-2];
        }
        y[len + 1] = y[len - 1];
        y[len] = y[len - 2] + ex[1] * h[len - 1];
        /* *ternary stochastic code book (-1, 0, +1)			 */
        if (ex[1] > 0.0f)
        {
            if (ex[0] > 0.0f)
            {
                for (i = len - 1; i > 1; i--)
                    y[i] = y[i-2] + h[i-1] + h[i];
            }
            else if(ex[0] < 0.0f)
            {
                for (i = len - 1; i > 1; i--)
                    y[i] = y[i-2] + h[i-1] - h[i];
            }
            else
            {
                for (i = len - 1; i > 1; i--)
                    y[i] = y[i-2] + h[i-1];
            }
        }
        else if (ex[1] < 0.0f)
        {
            if (ex[0] > 0.0f)
            {
                for (i = len - 1; i > 1; i--)
                    y[i] = y[i-2] - h[i-1] + h[i];
            }
            else if(ex[0] < 0.0f)
            {
                for (i = len - 1; i > 1; i--)
                    y[i] = y[i-2] - h[i-1] - h[i];
            }
            else
            {
                for (i = len - 1; i > 1; i--)
                    y[i] = y[i-2] - h[i-1];
            }
        }
        else
        {
            if (ex[0] > 0.0f)
            {
                for (i = len - 1; i > 1; i--)
                    y[i] = y[i-2] + h[i];
            }
            else if(ex[0] < 0.0f)
            {
                for (i = len - 1; i > 1; i--)
                    y[i] = y[i-2] - h[i];
            }
            else
            {
                for (i = len - 1; i > 1; i--)
                    y[i] = y[i-2];
            }
        }
        y[1] = ex[1] * h[0] + ex[0] * h[1];
        y[0] = ex[0] * h[0];
    }
    
    /**	Calculate correlation and energy:
    e0 = spectrum & pitch prediction residual
    y  = error weighting filtered code words
    
      \/\/\/  CELP's computations are focused in this correlation \/\/\/
      - For a 512 code book this correlation takes 4 MIPS!
    - Decimation?, Down-sample & decimate?, FEC codes?	*/
    
    cor = 0.0f;
    
    /* *End correct energy on subsequent code words:			 */
    if (lrintf(ex[0]) == 0 && lrintf(ex[1]) == 0 && !first)
    {
        py = y;
        for (i = 0; i < l; i++, py++)
        {
            cor += *py * st->e0[i];
        }
        eng = eng - st->y59save * st->y59save - st->y60save * st->y60save;
    }
    else
    {
        py = y;
        eng = 0.0f;
        for (i = 0; i < l; i++)
        {
            float temp = py[i];

            eng += temp * temp;
            cor += temp * st->e0[i];
        }
    }
    st->y59save = y[l - 2];
    st->y60save = y[l - 1];
    st->eng = eng;
    
    /*	Independent (open-loop) quantization of gain and match (index):	 */
    
    if (st->eng <= 0.0f)
        st->eng = 1.0f;
    cgain = cor / st->eng;
    *match = cor * cgain;
    
    return (cgain);
}

static void mexcite1(int l, celp_encoder_state *st)
{
    int i;
    
    /* *e1 = Euclidean norm of the first error signal		*/
    /* (note: the error signal array e0 is reused)		*/
    
    st->e1 = 1e-6f;
    for (i = 0; i < l; i++)
    {
        st->e0save[i] = st->e0[i];
        st->e1 += st->e0[i] * st->e0[i];
    }
}

static void mexcite2(int l, celp_encoder_state *st)
{
    int i;
    
    /* *ccor = crosscorrelation of the residual signals before 	*/
    /* *and after pitch prediction				*/
    /* *(note: the error signal array e0 is reused		*/
    
    st->ccor = 1e-6f;
    for (i = 0; i < l; i++)
        st->ccor += st->e0[i] * st->e0save[i];
    
    /* *normalize the crosscorrelation				*/
    
    st->ccor = st->ccor / st->e1;
}

static void mexcite3(float *cgain, celp_encoder_state *st)
{
    float scale;
    
    /* *square root crosscorrelation scaling			*/
    
    scale = (float)sqrt(fabs(st->ccor));
    
    /* *modify scale						*/
    
    if (scale < 0.2f)
        scale = 0.2f;
    else if (scale > 0.9f) 
        scale = scale * 1.4f;
    
    /* *modify the stochastic component				*/
    
    *cgain = *cgain * scale;
}

static void cbsearch(int l, float *v, int *cbindex, int *gindex, celp_encoder_state *st)
{
    int i, codeword, index, cblength;
    float emax, gain, gmax, err, *px, *pv;
    
    codeword = 2*MAXNCSIZE - 2;
    index = 1;
    cblength = st->cblength;
    if(st->fastgain == 1)
    {
        gain = fastcgain(&x[codeword], l, TRUE, st->inpulselen, &err, st);
        emax = err;
        gmax = gain;
        codeword -= 2;
        for (i = 1; i < cblength; i++)
        {
            gain = fastcgain(&x[codeword], l, FALSE, st->inpulselen, &err, st);
            codeword -= 2;
            if (err >= emax)
            {
                gmax = gain;
                emax = err;
                index = i + 1;
            }
        }
    }
    else
    {
        gain = cgain(&x[codeword], l, TRUE, st->inpulselen, &err, st);
        emax = err;
        gmax = gain;
        codeword -= 2;
        for (i = 1; i < cblength; i++)
        {
            gain = cgain(&x[codeword], l, FALSE, st->inpulselen, &err, st);
            codeword -= 2;
            if (err >= emax)
            {
                gmax = gain;
                emax = err;
                index = i + 1;
            }
        }
    }
    
    /*		*pointer to best code word				*/
    
    codeword = 2*(MAXNCSIZE - index);
    
    /*		*OPTIONAL (may be useful for integer DSPs)		*/
    /*		*given best code word, recompute its gain to		*/
    /*		*correct any accumulated errors in recursions		*/
    /*  gain[*cbindex-1] = cgain(&x[codeword], l, TRUE, l, &err[*cbindex-1], st); */
    
    /* *constrained excitation						*/
    mexcite3(&gmax, st);
    
    /*		*gain quantization, UNNECESSARY for closed-loop quant	*/
    
    gmax = gainencode(gmax, gindex);
    
    /*		*scale selected code word vector -> excitation array	*/
    /*		*call VDECODE?						*/
    px = x + codeword;
    pv = v;
    for (i = 0; i < l; i++, pv++, px++)
        *pv = gmax * *px;
    *cbindex = index;
}

static void impulse(int l, float *fce, celp_encoder_state *st)
{
    float d5[MAXNO+1];

    memset(st->h, 0, sizeof(st->h[0]) * l);
    memset(d5, 0, sizeof(d5[0]) * (MAXNO+1));
    st->h[0] = 1.0;

    polefilt10(fce, d5, st->h, l);
}

static void movefr(int n, float *a, float *b)
{
    int i;
    
    for (i = 0; i < n; i++)
        *b++ = *a++;
}


static void confg(float *s, int l, float *d1, float *d2, float *d3, float *d4, int isw1,
                  float *fci, float *fce, celp_encoder_state *st)
{
    int i;

    if (isw1 != 0)
        pitchvql(st->e0, l, d1, st->idb, st->bb);
    polefilt10(fci, d2, st->e0, l);
    
    for (i = 0; i < l; i++)
        st->e0[i] = s[i] - st->e0[i];
    
    zerofilt10(fci, d3, st->e0, l);
    polefilt10(fce, d4, st->e0, l);
}

void csub(float *s, float *v, int l, int *cbindex, int *gindex, celp_encoder_state *st,
          float *fci, int *pindex, int *tauptr, int *minptr)
{
    float fce[MAXNO+1];

    memset(fce, 0, sizeof(fce));
    bwexp10(st->gamma2, fci, fce);

    /* *find the intial error without pitch VQ		 	 */

    memset(st->e0, 0, sizeof(st->e0[0]) * l);
    confg(s, l, st->d1a, st->d2a, st->d3a, st->d4a, 0, fci, fce, st);
    movefr(MAXNO + 1, st->d2b, st->d2a);
    movefr(MAXNO + 1, st->d3b, st->d3a);
    movefr(MAXNO + 1, st->d4b, st->d4a);
    
    /* *find impulse response (h) of perceptual weighting filter	 */
    
    impulse(l, fce, st);
    
    /* *norm of the first error signal for const. exc.		 */
    
    mexcite1(l, st);
    
    /* *pitch (adaptive code book) search						 */
    
    psearch(l, pindex, st, tauptr, minptr, st->d1b);
    
    /* *find initial error with pitch VQ
    */
    memset(st->e0, 0, sizeof(st->e0[0]) * l);
    confg(s, l, st->d1a, st->d2a, st->d3a, st->d4a, 1, fci, fce, st);
    
    /* *norm of second error signal for const. exc.		 */
    
    mexcite2(l, st);
    
    /* *stochastic code book search 				 */
    
    cbsearch(l, v, cbindex, gindex, st);
    
    /* *update filter states 					 */
    
    movefr(l, v, st->e0);
    confg(s, l, st->d1b, st->d2b, st->d3b, st->d4b, 1, fci, fce, st);
    movefr(st->idb, st->d1b, st->d1a);
    movefr(MAXNO + 1, st->d2b, st->d2a);
    movefr(MAXNO + 1, st->d3b, st->d3a);
    movefr(MAXNO + 1, st->d4b, st->d4a);
}

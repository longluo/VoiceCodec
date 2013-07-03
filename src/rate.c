/*
  HawkVoice Direct Interface (HVDI) cross platform network voice library
  Copyright (C) 2001-2004 Phil Frisbie, Jr. (phil@hawksoft.com)

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

#ifndef MACOSX
#include <malloc.h>
#endif
#include <string.h>
#include <limits.h>
#include "hvdint.h"

/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained. 
 * Lance Norskog And Sundry Contributors are not responsible for 
 * the consequences of using this software.
 */

/*
 * Modified for use in HawkVoiceDI by Phil Frisbie, Jr.
 */

/*
 * Least Common Multiple Linear Interpolation 
 *
 * Find least common multiple of the two sample rates.
 * Construct the signal at the LCM by interpolating successive
 * input samples as straight lines.  Pull output samples from
 * this line at output rate.
 *
 * Of course, actually calculate only the output samples.
 *
 * LCM must be 32 bits or less.  Two prime number sample rates
 * between 32768 and 65535 will yield a 32-bit LCM, so this is 
 * stretching it.
 */

/*
 * Algorithm:
 *	
 *	Generate a master sample clock from the LCM of the two rates.
 *	Interpolate linearly along it.	Count up input and output skips.
 *
 *	Input:	 |inskip |		 |		 |		 |		 |
 *																		
 *																		
 *																		
 *	LCM:	 |	 |	 |	 |	 |	 |	 |	 |	 |	 |	 |
 *																		
 *																		
 *																		
 *	Output:  |	outskip  |			 |			 | 
 *
 *																		
 */

typedef struct hvdi_rate_st {
    long    lcmrate;		    /* least common multiple of rates */
    long    inskip, outskip;    /* LCM increments for I & O rates */
    long    total;
    long    intot, outtot;      /* total samples in terms of LCM rate */
    long    lastsamp;
} hvdi_rate_t;

/* here for linear interp.	might be useful for other things */

static long gcd(long a, long b) 
{
		if (b == 0)
				return a;
		else
				return gcd(b, a % b);
}

static long lcm(long a, long b) 
{
		return (a * b) / gcd(a, b);
}

/*
 * Prepare processing.
 */
HL_EXP hvdi_rate* HL_APIENTRY hvdiNewRate(int inrate, int outrate)
{
    hvdi_rate *rate;
    
    rate = malloc(sizeof(hvdi_rate));
    if(rate == NULL)
    {
        return NULL;
    }
    rate->lcmrate = lcm((long) inrate, (long) outrate);
    /* Cursory check for LCM overflow.	
    * If both rate are below 65k, there should be no problem.
    * 16 bits x 16 bits = 32 bits, which we can handle.
    */
    rate->inskip = rate->lcmrate / inrate;
    rate->outskip = rate->lcmrate / outrate; 
    rate->total = rate->intot = rate->outtot = 0;
    rate->lastsamp = 0;
    return rate;
}

/*
 * Processed signed short samples from ibuf to obuf.
 * Return number of samples processed.
 */

HL_EXP void HL_APIENTRY hvdiRateFlow(hvdi_rate *rate, short *ibuf, short *obuf, int *inlen, int *outlen)
{
	int len, done;
	short *istart = ibuf;
	long last;
		

		done = 0;

		if (rate->total == 0) {
				/* Emit first sample.  We know the fence posts meet. */
				*obuf = *ibuf++;
				/* Count up until have right input samples */
				rate->lastsamp = *obuf++;
				done = 1;
				rate->total = 1;
				/* advance to second output */
				rate->outtot += rate->outskip;
				/* advance input range to span next output */
				while ((rate->intot + rate->inskip) <= rate->outtot){
						rate->lastsamp = *ibuf++;
						rate->intot += rate->inskip;
				}
		}

		/* number of output samples the input can feed */

		len = (*inlen * rate->inskip) / rate->outskip;
		if (len > *outlen)
				len = *outlen;
		last = rate->lastsamp;
		for(; done < len; done++) {
				long osl;

				osl = last + ((long) ((long)(*ibuf) - last) *
						(rate->outtot - rate->intot)) / rate->inskip;
                if(osl > SHRT_MAX)
                {
                    osl = SHRT_MAX;
                }
                else if(osl < SHRT_MIN)
                {
                    osl = SHRT_MIN;
                }
				*obuf++ = (short)osl;
				/* advance to next output */
				rate->outtot += rate->outskip;
				/* advance input range to span next output */
				while ((rate->intot + rate->inskip) <= rate->outtot){
						last = *ibuf++;
						rate->intot += rate->inskip;
						if (ibuf - istart == *inlen)
								goto out;
				}
                /* long samples with high LCM's overrun counters! */
				if (rate->outtot == rate->intot)
						rate->outtot = rate->intot = 0;
		}
out:
		*inlen = ibuf - istart;
		*outlen = done;
		rate->lastsamp = last;
}

HL_EXP void HL_APIENTRY hvdiDeleteRate(hvdi_rate *rate)
{
    free(rate);
}

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
#ifndef HVDINT_H
#define HVDINT_H

#include "hvdi.h"
#include "ulaw/u-law.h"
#include "adpcm/adpcm.h"
#include "gsm/gsm.h"
#include "lpc/lpc.h"
#include "lpc10/lpc10.h"
#include "openlpc/openlpc.h"
#include "celp/celp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This was copied from nl.h so that it did not need to be included */

#if defined WIN32 || defined WIN64 || defined __i386__ || defined __alpha__
#define NL_LITTLE_ENDIAN
#endif

#ifdef NL_LITTLE_ENDIAN
#define nlSwaps(x) (unsigned short)(((((unsigned short)x) & 0x00ff) << 8) |\
                              ((((unsigned short)x) & 0xff00) >> 8))
#else
/* no conversion needed for big endian */
#define nlSwaps(x) (x)
#endif /* NL_LITTLE_ENDIAN */

/* macros for writing/reading packet buffers */
/* NOTE: these also endian swap the data as needed */
/* write* or read* (buffer *, count, data [, length]) */
#define writeByte(x, y, z)      (*(unsigned char *)&x[y++] = (unsigned char)z)
#define writeShort(x, y, z)     {*((unsigned short *)((unsigned char *)&x[y])) = nlSwaps(z); y += 2;}
#define readByte(x, y, z)       (z = *(unsigned char *)&x[y++])
#define readShort(x, y, z)      {z = nlSwaps(*(unsigned short *)((unsigned char *)&x[y])); y += 2;}
#define readSignedByte(x, y, z)       (z = *(char *)&x[y++])
#define readSignedShort(x, y, z)      {z = (short)nlSwaps(*(unsigned short *)((unsigned char *)&x[y])); y += 2;}

/* For encryption */
#define HVDI_ENCRYPT_BIT    0x0080  /* OR'd with the codec if encrypted */

/* For sequence number */
#define HVDI_SEQUENCE_BIT   0x0040  /* OR'd with the codec if sequence number is present */

/* globals */

typedef struct hvdi_state_st {
    int gsm_lpt;
    int celp_codebook;
    int celp_fast_gain;
    int sequence;
    int autoVOX;
    int VOXlevel;
    int VOXspeed;
    int comfortnoise;
    int noiselevel;
} hvdi_state_t;

extern volatile hvdi_state_t hvdi_state;

/* internal functions */
int hvdiSetDecoderCodec(unsigned char codec, hvdi_dec_state *st);
void hvdiResetDecoderCodec(hvdi_dec_state *st);


/* internal structures */

typedef struct hvdi_vox_st {
    int     rate;           /* HVDI_VOX_FAST, HVDI_VOX_MEDIUM, or HVDI_VOX_SLOW */
    int     noisethreshold; /* The actual threshold used by hvdiVOX */
    int     samplecount;    /* init to 0; used internally by hvdiVOX */
} hvdi_vox_t;

typedef struct hvdi_enc_state_st {
    unsigned char   codec;      /* the codec used with the last packet */
    unsigned short  sequence;   /* the sequence number of the last packet */
    void /*&null&*/ *state;     /* the codec state */
    hvdi_vox_t /*&null&*/*vox;  /* the VOX structure for auto VOX */
} hvdi_enc_state_t;

typedef struct hvdi_dec_state_st {
    unsigned char   codec;      /* the codec used with the last packet */
    unsigned short  sequence;   /* the sequence number of the last packet */
    void /*&null&*/ *state;     /* the codec state */
} hvdi_dec_state_t;

typedef struct hvdi_agc_st {
    unsigned int    sample_max;
    int             counter;
    long            igain;
    int             ipeak;
    int             silence_counter;
} hvdi_agc_t;

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* HVDINT_H */


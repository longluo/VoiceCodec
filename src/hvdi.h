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

#ifndef HVDI_H
#define HVDI_H

#include "hawklib.h"
#include "hcrypt.h"

#ifdef __cplusplus
extern "C" {
#endif


#define HVDI_MAJOR_VERSION 0
#define HVDI_MINOR_VERSION 91
#define HVDI_VERSION_STRING "HVDI 0.91 beta"


#ifndef NL_INVALID
#define NL_INVALID              (-1)
#define NL_FALSE                (0)
#define NL_TRUE                 (1)
#endif

#ifdef HVDI_OLD_API
#define hvdiCreateEncoderState  hvdiNewEncState
#define hvdiCreateDecoderState  hvdiNewDecState
#define hvdiFreeEncoderState    hvdiDeleteEncState
#define hvdiFreeDecoderState    hvdiDeleteDecState
#define hvdiSetCodec(x, y)      hvdiEncStateSetCodec(y, x)
#define hvdiIsVoicePacket       hvdiPacketIsVoice       
#define hvdiDecodePacket        hvdiPacketDecode
#define hvdiEncodePacket(a, b, c, d, e, f) hvdiPacketEncode(c, b, a, d, e, f)
#endif

/* We will use HVDI or hvdi to prefix all HawkVoiceDI defines and functions */

typedef struct hvdi_vox_st hvdi_vox;
typedef struct hvdi_enc_state_st hvdi_enc_state;
typedef struct hvdi_dec_state_st hvdi_dec_state;
typedef struct hvdi_agc_st hvdi_agc;
typedef struct hvdi_rate_st hvdi_rate;


/* The basic codecs, from hawkvoice.h */
#define HV_2_4K_CODEC       0x0001  /* LPC-10 2.4 Kbps codec */
#define HV_4_8K_CODEC       0x0002  /* LPC 4.8 Kbps codec */
#define HV_13_2K_CODEC      0x0003  /* GSM 13.2 Kbps codec */
#define HV_32K_CODEC        0x0004  /* Intel/DVI ADPCM 32 Kbps codec */
#define HV_64K_CODEC        0x0005  /* G.711 u-law 64 Kbps codec */
#define HV_1_4K_CODEC       0x0006  /* OpenLPC 1.4 Kbps codec */
#define HV_1_8K_CODEC       0x0007  /* OpenLPC 1.8 Kbps codec */
#define HV_4_5K_CODEC       0x0008  /* CELP 4.5 Kbps codec */
#define HV_3_0K_CODEC       0x0009  /* CELP 3.0 Kbps codec */
#define HV_2_3K_CODEC       0x000a  /* CELP 2.3 Kbps codec */
#define HV_VBR_2_4K_CODEC   0x000b  /* Variable Bit Rate LPC-10 2.4 Kbps max codec*/

#define HV_SILENCE_CODEC    0x001f  /* Silence codec, used internally */

/* Alternate codec names */
#define HV_LPC10_CODEC      HV_2_4K_CODEC
#define HV_LPC_CODEC        HV_4_8K_CODEC
#define HV_GSM_CODEC        HV_13_2K_CODEC
#define HV_ADPCM_32_CODEC   HV_32K_CODEC
#define HV_PCM_64_CODEC     HV_64K_CODEC
#define HV_G_711_CODEC      HV_64K_CODEC
#define HV_ULAW_CODEC       HV_64K_CODEC
#define HV_LPC_1_4_CODEC    HV_1_4K_CODEC
#define HV_LPC_1_8_CODEC    HV_1_8K_CODEC
#define HV_CELP_4_5_CODEC   HV_4_5K_CODEC
#define HV_CELP_3_0_CODEC   HV_3_0K_CODEC
#define HV_CELP_2_3_CODEC   HV_2_3K_CODEC
#define HV_VBR_LPC10_CODEC  HV_VBR_2_4K_CODEC

/* VOX options */
/* How many samples of silence to wait after voice stops. */
/* You can use any value, these are just for reference. */
#define HVDI_VOX_FAST       4000    /* 1/2 second */
#define HVDI_VOX_MEDIUM     8000    /* 1 second */
#define HVDI_VOX_SLOW      12000    /* 1 1/2 seconds */

/* hvdiHint options*/
#define HVDI_NORMAL         0x0001  /* Normal encoding/decoding speed, best quality, arg ignored */
#define HVDI_FAST           0x0002  /* Faster encoding/decoding, some loss of quality 
                                       with some codecs, arg ignored */
#define HVDI_FASTEST        0x0003  /* Fastest possible encoding/decoding, more loss of quality,
                                       arg ignored */
#define HVDI_CELP_CODEBOOK  0x0004  /* Directly change the CELP encoding codebook length, arg
                                       valid range 32 to 256 */
#define HVDI_SEQUENCE       0x0005  /* Determines if the sequence number is sent in the packet.
                                       To disable, arg = 0, to enable (default), arg != 0 */
#define HVDI_AUTO_VOX       0x0006  /* Enables automatic VOX processing inside hvdiPacketEncode.
                                       Default VOX setting is 300 unless first set by HVD_VOX_LEVEL */
#define HVDI_VOX_LEVEL      0x0007  /* Sets the threshhold level when using HVDI_AUTO_VOX, arg
                                       valid range 0 to 1000, default 300 */
#define HVDI_VOX_SPEED      0x0008  /* Sets the VOX speed, default HVDI_VOX_FAST */
#define HVDI_COMFORT_NOISE  0x0009  /* Enables sending silence packets by the encoder, and creates
                                       white noise in the decoder */
#define HVDI_NOISE_LEVEL    0x000a  /* Sets the decoder comfort noise level, 0 to 1000, default 100 */

/* HawkVoiceDI API */

HL_EXP hvdi_enc_state* HL_APIENTRY hvdiNewEncState(void);

HL_EXP hvdi_dec_state* HL_APIENTRY hvdiNewDecState(void);

HL_EXP void      HL_APIENTRY hvdiDeleteEncState(hvdi_enc_state *state);

HL_EXP void      HL_APIENTRY hvdiDeleteDecState(hvdi_dec_state *state);

HL_EXP int       HL_APIENTRY hvdiEncStateSetCodec(hvdi_enc_state *state, unsigned char codec);

HL_EXP unsigned char HL_APIENTRY hvdiDecStateGetCodec(hvdi_dec_state *state);

HL_EXP int       HL_APIENTRY hvdiPacketIsVoice(unsigned char *packet, int length);

HL_EXP int       HL_APIENTRY hvdiPacketDecode(unsigned char *packet, int paclen, short *buffer,
                                              int buflen, hcrypt_key *key, hvdi_dec_state *state);

HL_EXP int       HL_APIENTRY hvdiPacketEncode(unsigned char *packet, int buflen, short *buffer,
                                              int paclen, hcrypt_key *key, hvdi_enc_state *state);

HL_EXP hvdi_vox* HL_APIENTRY hvdiNewVOX(int vox_speed, int noisethreshold);

HL_EXP int       HL_APIENTRY hvdiVOX(hvdi_vox *vox, short *buffer, int buflen);

HL_EXP void      HL_APIENTRY hvdiDeleteVOX(hvdi_vox *vox);

HL_EXP hvdi_rate* HL_APIENTRY hvdiNewRate(int inrate, int outrate);

HL_EXP void      HL_APIENTRY hvdiRateFlow(hvdi_rate *rate, short *inbuf, short *outbuf, int *inlen, int *outlen);

HL_EXP void      HL_APIENTRY hvdiDeleteRate(hvdi_rate *rate);

HL_EXP hvdi_agc* HL_APIENTRY hvdiNewAGC(float level);

HL_EXP void      HL_APIENTRY hvdiAGC(hvdi_agc *agc, short *buffer, int len);

HL_EXP void      HL_APIENTRY hvdiDeleteAGC(hvdi_agc *agc);

HL_EXP void      HL_APIENTRY hvdiMix(short *outbuf, short **inbuf, int number, int inlen);

HL_EXP void      HL_APIENTRY hvdiHint(int name, int arg);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* HVDI_H */


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

#ifndef CELP_H
#define CELP_H

#ifdef __cplusplus
extern "C" {
#endif


/* the speech frame size must be between 240 and 480 samples, AND be a multiple of 4 */
#define CELP_4_5_FRAMESIZE          240
#define CELP_3_0_FRAMESIZE          360
#define CELP_2_3_FRAMESIZE          480

#define CELP_ENCODED_FRAME_SIZE     17

/* CELP encoder options */
/* value of 1 for enable, 0 for disable */
#define CELP_FAST_GAIN      1
#define CELP_CODEBOOK_LEN   2

/* celp_set_codebook_len is obsolete, use celp_set_encoder_option */
#define celp_set_codebook_len(st, len) celp_set_encoder_option(st, CELP_CODEBOOK_LEN, len)

typedef struct celp_enc_state celp_encoder_state;
typedef struct celp_dec_state celp_decoder_state;

celp_encoder_state *create_celp_encoder_state();
void init_celp_encoder_state(celp_encoder_state *st, int framesize);
int  celp_encode(const short *in, unsigned char *out, celp_encoder_state *st);
void celp_set_encoder_option(celp_encoder_state *st, int option, int value);
void destroy_celp_encoder_state(celp_encoder_state *st);


celp_decoder_state *create_celp_decoder_state();
void init_celp_decoder_state(celp_decoder_state *st, int framesize);
int  celp_decode(const unsigned char *in, short *out, celp_decoder_state *st);
void destroy_celp_decoder_state(celp_decoder_state *st);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* CELP_H */

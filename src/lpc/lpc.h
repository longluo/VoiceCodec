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

#ifndef LPC_H
#define LPC_H

#ifdef __cplusplus
extern "C" {
#endif

#define LPC_SAMPLES_PER_FRAME	    160
#define LPC_ENCODED_FRAME_SIZE      12

typedef struct lpc_e_state lpc_encoder_state;
typedef struct lpc_d_state lpc_decoder_state;

lpc_encoder_state *create_lpc_encoder_state(void);
void init_lpc_encoder_state(lpc_encoder_state *st);
int  lpc_encode(const short *in, unsigned char *out, lpc_encoder_state *st);
void destroy_lpc_encoder_state(lpc_encoder_state *st);

lpc_decoder_state *create_lpc_decoder_state(void);
void init_lpc_decoder_state(lpc_decoder_state *st);
int  lpc_decode(unsigned char *in, short *out, lpc_decoder_state *st);
void destroy_lpc_decoder_state(lpc_decoder_state *st);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* LPC_H */

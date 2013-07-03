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
#include "hvdint.h"

static int openlpcencode(short *in, unsigned char *out, int buflen, int paclen, void *state, int framesize)
{
    if(paclen >= (int)(buflen / framesize
                        * OPENLPC_ENCODED_FRAME_SIZE))
    {
        openlpc_encoder_state *st = (openlpc_encoder_state *)state;
        int         len = 0;
        int         i;

        /* make sure buflen is a multiple of the frame length */
        buflen = ((int)(buflen / framesize) * framesize);

        for(i = 0;i<buflen;i+=framesize)
        {
            len += openlpc_encode(&in[i], &out[len], st);
        }

        return len;
    }
    return 0;
}

static int lpc10encode(short *in, unsigned char *out, int buflen, int paclen, void *state)
{
    if(paclen >= (int)(buflen / LPC10_SAMPLES_PER_FRAME
                        * LPC10_ENCODED_FRAME_SIZE))
    {
        lpc10_encoder_state *st = (lpc10_encoder_state *)state;
        int         len = 0;
        int         i;

        /* make sure buflen is a multiple of the frame length */
        buflen = ((int)(buflen / LPC10_SAMPLES_PER_FRAME) * LPC10_SAMPLES_PER_FRAME);

        /* the LPC10 codec needs to process in LPC10_SAMPLES_PER_FRAME chunks */
        for(i = 0;i<buflen;i+=LPC10_SAMPLES_PER_FRAME)
        {
            len += lpc10_encode(&in[i], &out[len], st);
        }

        return len;
    }
    return 0;
}

static int lpcencode(short *in, unsigned char *out, int buflen, int paclen, void *state)
{
    if(paclen >= (int)(buflen / LPC_SAMPLES_PER_FRAME
                        * LPC_ENCODED_FRAME_SIZE))
    {
        lpc_encoder_state *st = (lpc_encoder_state *)state;
        int         len = 0;
        int         i;

        /* make sure buflen is a multiple of the frame length */
        buflen = ((int)(buflen / LPC_SAMPLES_PER_FRAME) * LPC_SAMPLES_PER_FRAME);

        /* the LPC codec needs to process in LPC_FRAMESIZE chunks */
        for(i = 0;i<buflen;i+=LPC_SAMPLES_PER_FRAME)
        {
            len += lpc_encode(&in[i], &out[len], st);
        }

        return len;
    }
    return 0;
}

static int gsmencode(short *in, unsigned char *out, int buflen, int paclen, void *state)
{
    if(paclen >= (int)(buflen / GSM_SAMPLES_PER_FRAME
                        * GSM_ENCODED_FRAME_SIZE))
    {
        struct gsm_state    *st = (struct gsm_state *)state;
        int                 len = 0;
        int                 i;
        int                 gsm_lpt = hvdi_state.gsm_lpt;

        /* make sure buflen is a multiple of the frame length */
        buflen = ((int)(buflen / GSM_SAMPLES_PER_FRAME) * GSM_SAMPLES_PER_FRAME);

        /* set the GSM_OPT_LTP_CUT option */
        (void)gsm_option(st, GSM_OPT_LTP_CUT, &gsm_lpt);

        /* the GSM codec needs to process in 160 sample chunks */
        for(i = 0;i<buflen;i+=GSM_SAMPLES_PER_FRAME)
        {
            len += gsm_encode(st, &in[i], &out[len]);
        }

        return len;
    }
    return 0;
}

static int adpcmencode(short *in, unsigned char *out, int buflen, int paclen, void *state)
{
    struct adpcm_state  *st = (struct adpcm_state *)state;

    if(paclen >= (int)(buflen / 2 + sizeof(st->valprev) + sizeof(st->index)))
    {
        int     len;
        int     count = 0;

        /* write out the previous codec state */
        writeShort(out, count, st->valprev);
        writeByte(out, count, st->index);

        len = count;
        len += adpcm_coder(in, &out[count], buflen, st);

        return len;
    }
    return 0;
}

static int ulawencode(short *in, unsigned char *out, int buflen, int paclen)
{

    if(paclen >= buflen)
    {
        return ulawEncode(in, out, buflen);
    }
    return 0;
}

static int celpencode(short *in, unsigned char *out, int buflen, int paclen, void *state, int framesize)
{
    
    if(paclen >= (int)(buflen / framesize
                        * CELP_ENCODED_FRAME_SIZE))
    {
        celp_encoder_state  *st = (celp_encoder_state *)state;
        int                 len = 0;
        int                 i;

        /* make sure buflen is a multiple of the frame length */
        buflen = ((int)(buflen / framesize) * framesize);

        /* set the options */
        celp_set_encoder_option(st, CELP_CODEBOOK_LEN, hvdi_state.celp_codebook);
        celp_set_encoder_option(st, CELP_FAST_GAIN, hvdi_state.celp_fast_gain);

        for(i = 0;i<buflen;i+=framesize)
        {
            len += celp_encode(&in[i], &out[len], st);
        }

        return len;
    }
    return 0;
}

static int vbrlpc10encode(short *in, unsigned char *out, int buflen, int paclen, void *state)
{
    if(paclen >= (int)(buflen / LPC10_SAMPLES_PER_FRAME
                        * LPC10_ENCODED_FRAME_SIZE))
    {
        lpc10_encoder_state *st = (lpc10_encoder_state *)state;
        int         len = 0;
        int         i;

        /* make sure buflen is a multiple of the frame length */
        buflen = ((int)(buflen / LPC10_SAMPLES_PER_FRAME) * LPC10_SAMPLES_PER_FRAME);

        /* the VBR-LPC-10 codec needs to process in VBR_LPC10_SAMPLES_PER_FRAME chunks */
        for(i = 0;i<buflen;i+=LPC10_SAMPLES_PER_FRAME)
        {
            len += vbr_lpc10_encode(&in[i], &out[len], st);
        }

        return len;
    }
    return 0;
}

HL_EXP int HL_APIENTRY hvdiPacketEncode(unsigned char *packet, int buflen, short *buffer,
                                              int paclen, hcrypt_key *key, hvdi_enc_state *state)
{
    int             len = 0;
    int             count = 0;
    int             result;
    unsigned char   *data;
    unsigned char   codec = state->codec;

    if(paclen < 3)
    {
        return 0;
    }
    /* check for auto VOX state */
    if(hvdi_state.autoVOX != 0)
    {
        /* check for NULL VOX state */
        if(state->vox == NULL)
        {
            state->vox = hvdiNewVOX(hvdi_state.VOXspeed, 300);
        }
        /* update from global state */
        state->vox->rate = hvdi_state.VOXspeed;
        state->vox->noisethreshold = hvdi_state.VOXlevel;
        /* now run VOX */
        if(hvdiVOX(state->vox, buffer, buflen) == 0)
        {
            if(hvdi_state.comfortnoise != 0)
            {
                /* encode a silence packet */
                codec = (unsigned char)HV_SILENCE_CODEC;
                if(hvdi_state.sequence != 0)
                {
                    codec |= HVDI_SEQUENCE_BIT;
                }
                if(key != NULL)
                {
                    codec |= HVDI_ENCRYPT_BIT;
                }
                /* write the header */
                writeByte(packet, count, codec);
                if(hvdi_state.sequence != 0)
                {
                    writeShort(packet, count, state->sequence);
                }
                writeShort(packet, count, buflen);
                /* We are going to skip encryption for silence packets */
                /* but we will authenticate */
                if(key != NULL)
                {
                    /* sign the entire packet */
                    hcryptSignPacket(packet, count, key);
                    count += 4;
                }
                state->sequence++;
                return count;
            }
            else
            {
                return 0;
            }
        }
    }
    if(key != NULL)
    {
        codec |= HVDI_ENCRYPT_BIT;
    }
    if(hvdi_state.sequence != 0)
    {
        codec |= HVDI_SEQUENCE_BIT;
    }
    /* write the header */
    writeByte(packet, count, codec);
    if(hvdi_state.sequence != 0)
    {
        writeShort(packet, count, state->sequence);
    }
    len = count;
    /* data points to the start of the encoded speech */
    data = &packet[count];

    /* encode the speech */
    switch (state->codec) {
    case HV_1_4K_CODEC:
        result = openlpcencode(buffer, data, buflen, paclen, state->state, OPENLPC_FRAMESIZE_1_4);
        break;

    case HV_1_8K_CODEC:
        result = openlpcencode(buffer, data, buflen, paclen, state->state, OPENLPC_FRAMESIZE_1_8);
        break;

    case HV_2_4K_CODEC:
        result = lpc10encode(buffer, data, buflen, paclen, state->state);
        break;

    case HV_4_8K_CODEC:
        result = lpcencode(buffer, data, buflen, paclen, state->state);
        break;

    case HV_13_2K_CODEC:
        result = gsmencode(buffer, data, buflen, paclen, state->state);
        break;

    case HV_32K_CODEC:
        result = adpcmencode(buffer, data, buflen, paclen, state->state);
        break;

    case HV_64K_CODEC:
        result = ulawencode(buffer, data, buflen, paclen);
        break;

    case HV_4_5K_CODEC:
        result = celpencode(buffer, data, buflen, paclen, state->state, CELP_4_5_FRAMESIZE);
        break;

    case HV_3_0K_CODEC:
        result = celpencode(buffer, data, buflen, paclen, state->state, CELP_3_0_FRAMESIZE);
        break;

    case HV_2_3K_CODEC:
        result = celpencode(buffer, data, buflen, paclen, state->state, CELP_2_3_FRAMESIZE);
        break;

    case HV_VBR_2_4K_CODEC:
        result = vbrlpc10encode(buffer, data, buflen, paclen, state->state);
        break;

    default:
        /* invalid codec */
        return 0;
    }

    if(result <= 0)
    {
        return 0;
    }
    len += result;

    /* encrypt the packet if needed */
    if(key != NULL)
    {
        if((len + 4) > paclen)
        {
            /* packet not long enough */
            return 0;
        }
        /* encrypt only the encoded speech */
        hcryptEncryptPacket(data, data, result, key);
        /* sign the entire packet */
        hcryptSignPacket(packet, len, key);
        len += 4;
    }
    state->sequence++;

    return len;
}



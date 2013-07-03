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
#ifdef _MSC_VER
#pragma warning (disable:4001) /* to disable warning in <math.h> */
#endif

#ifndef MACOSX
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "hvdint.h"

#ifndef min
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#endif


volatile hvdi_state_t hvdi_state = {0, 256, 0, 1, 0, 1448, HVDI_VOX_FAST, 0, 304};

/*
    This is similar to HV_VOICE_DATA, but it is a little more compact
    because it does not include the target.
    
    HVDI_VOICE_DATA- This is a voice data packet.
        NLubyte     type of codec OR'd with encryption and sequence bits
        NLushort    packet sequence number (optional)
        NLvoid      *data
*/

/* internal functions */

static void hvdiFreeEncoder(hvdi_enc_state *st)
{
    if(st->state == NULL)
    {
        return;
    }
    switch(st->codec){
    case HV_2_4K_CODEC:
    case HV_VBR_2_4K_CODEC:
        destroy_lpc10_encoder_state((lpc10_encoder_state *)st->state);
        break;

    case HV_4_8K_CODEC:
        destroy_lpc_encoder_state((lpc_encoder_state *)st->state);
        break;

    case HV_13_2K_CODEC:
        gsm_destroy((struct gsm_state *)st->state);
        break;

    case HV_32K_CODEC:
        free(st->state);
        break;

    case HV_1_4K_CODEC:
    case HV_1_8K_CODEC:
        destroy_openlpc_encoder_state((openlpc_encoder_state *)st->state);
        break;

    case HV_4_5K_CODEC:
    case HV_3_0K_CODEC:
    case HV_2_3K_CODEC:
        destroy_celp_encoder_state((celp_encoder_state *)st->state);
        break;

    default:
        break;
    }
    st->state = NULL;
}

static void hvdiFreeDecoder(hvdi_dec_state *st)
{
    if(st->state == NULL)
    {
        return;
    }
    switch(st->codec){
    case HV_2_4K_CODEC:
    case HV_VBR_2_4K_CODEC:
        destroy_lpc10_decoder_state((lpc10_decoder_state *)st->state);
        break;

    case HV_4_8K_CODEC:
        destroy_lpc_decoder_state((lpc_decoder_state *)st->state);
        break;

    case HV_13_2K_CODEC:
        gsm_destroy((struct gsm_state *)st->state);
        break;

    case HV_1_4K_CODEC:
    case HV_1_8K_CODEC:
        destroy_openlpc_decoder_state((openlpc_decoder_state *)st->state);
        break;

    case HV_4_5K_CODEC:
    case HV_3_0K_CODEC:
    case HV_2_3K_CODEC:
        destroy_celp_decoder_state((celp_decoder_state *)st->state);
        break;

    default:
        break;
    }
    st->state = NULL;
}

int hvdiSetDecoderCodec(unsigned char codec, hvdi_dec_state *st)
{
    /* free the old state */
    hvdiFreeDecoder(st);

    /* create the new state */
    switch(codec){
    case HV_2_4K_CODEC:
    case HV_VBR_2_4K_CODEC:
        st->state = create_lpc10_decoder_state();
        if(st->state == NULL)
        {
            return NL_FALSE;
        }
        init_lpc10_decoder_state((lpc10_decoder_state *)st->state);
        break;

    case HV_4_8K_CODEC:
        st->state = create_lpc_decoder_state();
        if(st->state == NULL)
        {
            return NL_FALSE;
        }
        init_lpc_decoder_state((lpc_decoder_state *)st->state);
        break;

    case HV_13_2K_CODEC:
        st->state = gsm_create();
        if(st->state == NULL)
        {
            return NL_FALSE;
        }
        break;

    case HV_1_4K_CODEC:
        st->state = create_openlpc_decoder_state();
        if(st->state == NULL)
        {
            return NL_FALSE;
        }
        init_openlpc_decoder_state((openlpc_decoder_state *)st->state, OPENLPC_FRAMESIZE_1_4);
        break;

    case HV_1_8K_CODEC:
        st->state = create_openlpc_decoder_state();
        if(st->state == NULL)
        {
            return NL_FALSE;
        }
        init_openlpc_decoder_state((openlpc_decoder_state *)st->state, OPENLPC_FRAMESIZE_1_8);
        break;

    case HV_4_5K_CODEC:
        st->state = create_celp_decoder_state();
        init_celp_decoder_state(st->state, CELP_4_5_FRAMESIZE);
        if(st->state == NULL)
        {
            return NL_FALSE;
        }
        break;

    case HV_3_0K_CODEC:
        st->state = create_celp_decoder_state();
        init_celp_decoder_state(st->state, CELP_3_0_FRAMESIZE);
        if(st->state == NULL)
        {
            return NL_FALSE;
        }
        break;

    case HV_2_3K_CODEC:
        st->state = create_celp_decoder_state();
        init_celp_decoder_state(st->state, CELP_2_3_FRAMESIZE);
        if(st->state == NULL)
        {
            return NL_FALSE;
        }
        break;

    default:
        break;
    }

    st->codec = codec;
    return NL_TRUE;
}

void hvdiResetDecoderCodec(hvdi_dec_state *st)
{
    /* reset the state */
    switch(st->codec){
    case HV_2_4K_CODEC:
    case HV_VBR_2_4K_CODEC:
        init_lpc10_decoder_state((lpc10_decoder_state *)st->state);
        break;

    case HV_4_8K_CODEC:
        init_lpc_decoder_state((lpc_decoder_state *)st->state);
        break;

    case HV_13_2K_CODEC:
        gsm_destroy((struct gsm_state *)st->state);
        st->state = gsm_create();
        break;

    case HV_1_4K_CODEC:
        init_openlpc_decoder_state((openlpc_decoder_state *)st->state, OPENLPC_FRAMESIZE_1_4);
        break;

    case HV_1_8K_CODEC:
        init_openlpc_decoder_state((openlpc_decoder_state *)st->state, OPENLPC_FRAMESIZE_1_8);
        break;

    case HV_4_5K_CODEC:
        init_celp_decoder_state((celp_decoder_state *)st->state, CELP_4_5_FRAMESIZE);
        break;

    case HV_3_0K_CODEC:
        init_celp_decoder_state((celp_decoder_state *)st->state, CELP_3_0_FRAMESIZE);
        break;

    case HV_2_3K_CODEC:
        init_celp_decoder_state((celp_decoder_state *)st->state, CELP_2_3_FRAMESIZE);
        break;

    default:
        break;
    }
}

/* HawkVoiceDI API */

HL_EXP hvdi_enc_state* HL_APIENTRY hvdiNewEncState(void)
{
    hvdi_enc_state *state;

    state = calloc(1, sizeof(hvdi_enc_state));

    return state;
}

HL_EXP hvdi_dec_state* HL_APIENTRY hvdiNewDecState(void)
{
    hvdi_dec_state *state;

    state = calloc(1, sizeof(hvdi_dec_state));

    return state;
}

HL_EXP void HL_APIENTRY hvdiDeleteEncState(hvdi_enc_state *st)
{
    if(st != NULL)
    {
        /* free the codec state */
        hvdiFreeEncoder(st);

        /* free VOX if used */
        if(st->vox != NULL)
        {
            free(st->vox);
            st->vox = NULL;
        }
        /* free the state structure */
        free(st);
    }
}

HL_EXP void HL_APIENTRY hvdiDeleteDecState(hvdi_dec_state *st)
{
    if(st != NULL)
    {
        /* free the codec state */
        hvdiFreeDecoder(st);

        /* free the state structure */
        free(st);
    }
}

HL_EXP int HL_APIENTRY hvdiEncStateSetCodec(hvdi_enc_state *st, unsigned char codec)
{
    int   frame = 0;

    /* free the old state */
    hvdiFreeEncoder(st);

    /* create the new state */
    switch(codec){
    case HV_2_4K_CODEC:
    case HV_VBR_2_4K_CODEC:
        st->state = create_lpc10_encoder_state();
        if(st->state == NULL)
        {
            return NL_INVALID;
        }
        init_lpc10_encoder_state((lpc10_encoder_state *)st->state);
        frame = LPC10_SAMPLES_PER_FRAME;
        break;

    case HV_4_8K_CODEC:
        st->state = create_lpc_encoder_state();
        if(st->state == NULL)
        {
            return NL_INVALID;
        }
        init_lpc_encoder_state((lpc_encoder_state *)st->state);
        frame = LPC_SAMPLES_PER_FRAME;
        break;

    case HV_13_2K_CODEC:
        st->state = gsm_create();
        if(st->state == NULL)
        {
            return NL_INVALID;
        }
        frame = GSM_SAMPLES_PER_FRAME;
        break;

    case HV_32K_CODEC:
        st->state = calloc(1, sizeof(struct adpcm_state));
        if(st->state == NULL)
        {
            return NL_INVALID;
        }
        break;

    case HV_1_4K_CODEC:
        st->state = create_openlpc_encoder_state();
        if(st->state == NULL)
        {
            return NL_INVALID;
        }
        init_openlpc_encoder_state((openlpc_encoder_state *)st->state, OPENLPC_FRAMESIZE_1_4);
        frame = OPENLPC_FRAMESIZE_1_4;
        break;

    case HV_1_8K_CODEC:
        st->state = create_openlpc_encoder_state();
        if(st->state == NULL)
        {
            return NL_INVALID;
        }
        init_openlpc_encoder_state((openlpc_encoder_state *)st->state, OPENLPC_FRAMESIZE_1_8);
        frame = OPENLPC_FRAMESIZE_1_8;
        break;

    case HV_4_5K_CODEC:
        st->state = create_celp_encoder_state();
        init_celp_encoder_state(st->state, CELP_4_5_FRAMESIZE);
        if(st->state == NULL)
        {
            return NL_INVALID;
        }
        frame = CELP_4_5_FRAMESIZE;
        break;

    case HV_3_0K_CODEC:
        st->state = create_celp_encoder_state();
        init_celp_encoder_state(st->state, CELP_3_0_FRAMESIZE);
        if(st->state == NULL)
        {
            return NL_INVALID;
        }
        frame = CELP_3_0_FRAMESIZE;
        break;

    case HV_2_3K_CODEC:
        st->state = create_celp_encoder_state();
        init_celp_encoder_state(st->state, CELP_2_3_FRAMESIZE);
        if(st->state == NULL)
        {
            return NL_INVALID;
        }
        frame = CELP_2_3_FRAMESIZE;
        break;

    default:
        break;
    }

    st->codec = codec;

    return frame;
}

HL_EXP unsigned char HL_APIENTRY hvdiDecStateGetCodec(hvdi_dec_state *state)
{
    return state->codec;
}

HL_EXP int HL_APIENTRY hvdiPacketIsVoice(unsigned char *packet, int paclen)
{
    int             count = 0;
    unsigned char   codec;
    unsigned short  sequence;

    /* paclen must be at least 3 for the header */
    if(paclen < 3)
    {
        return NL_FALSE;
    }
    /* read the packet header */
    readByte(packet, count, codec);
    if((codec&HVDI_SEQUENCE_BIT) != (unsigned char)0)
    {
        readShort(packet, count, sequence);
    }

    if((codec&HVDI_ENCRYPT_BIT) != (unsigned char)0)
    {
        /* adjust the paclen to remove the 4 byte MAC */
        paclen -= 4;
    }

    /* remove HVDI_ENCRYPT_BIT and HVDI_SEQUENCE_BIT */
    codec &= 0x3F;
    paclen -= count;

    /* check for valid codec and length */
    switch(codec){
    case HV_2_4K_CODEC:
        if((paclen % LPC10_ENCODED_FRAME_SIZE) != 0)
        {
            return NL_FALSE;
        }
        break;

    case HV_4_8K_CODEC:
        if((paclen % LPC_ENCODED_FRAME_SIZE) != 0)
        {
            return NL_FALSE;
        }
        break;

    case HV_13_2K_CODEC:
        if((paclen % GSM_ENCODED_FRAME_SIZE) != 0)
        {
            return NL_FALSE;
        }
        break;

    case HV_32K_CODEC:
    case HV_64K_CODEC:
        /* no real size limitation hear */
        break;

    case HV_1_4K_CODEC:
    case HV_1_8K_CODEC:
        if((paclen % OPENLPC_ENCODED_FRAME_SIZE) != 0)
        {
            return NL_FALSE;
        }
        break;

    case HV_4_5K_CODEC:
    case HV_3_0K_CODEC:
    case HV_2_3K_CODEC:
        if((paclen % CELP_ENCODED_FRAME_SIZE) != 0)
        {
            return NL_FALSE;
        }
        break;

    case HV_SILENCE_CODEC:
        if(paclen != 2)
        {
            return NL_FALSE;
        }
        break;

    case HV_VBR_2_4K_CODEC:
        /* we need a way to check variable length packets */
        break;

    default:
        return NL_FALSE;
    }

    return NL_TRUE;
}

/* hvdiDecodePacket() is in decpacket.c */

/* hvdiEncodePacket() is in encpacket.c */

/*
*  Basic VOX algorithm adapted from SpeakFreely
*
*  Returns 1 if buffer passes VOX, and 0 if it does not
*/

HL_EXP hvdi_vox* HL_APIENTRY hvdiNewVOX(int voxspeed, int noisethreshold)
{
    hvdi_vox *vox;

    vox = malloc(sizeof(hvdi_vox));
    if(vox == NULL)
    {
        return NULL;
    }
    vox->rate = voxspeed;
    vox->noisethreshold = (int) exp(log(32767.0) * ((1000 - noisethreshold) / 1000.0));
    vox->samplecount = 0;

    return vox;
}

HL_EXP int HL_APIENTRY hvdiVOX(hvdi_vox *vox, short *buffer, int buflen)
{
    int     i;
    long    level = 0;

    for(i=0;i<buflen;i++)
    {
        long sample = buffer[i];

        if(sample < 0)
        {
            level -= sample;
        }
        else
        {
            level += sample;
        }

    }
    level /= buflen;
    if(level < vox->noisethreshold)
    {
        if (vox->samplecount <= 0)
        {
            return 0;
        }
        vox->samplecount -= buflen;
    }
    else
    {
        vox->samplecount = vox->rate;
    }
    return 1;
}

HL_EXP void HL_APIENTRY hvdiDeleteVOX(hvdi_vox *vox)
{
    free(vox);
}

/*
*  This AGC algorithm was taken from isdn2h323 (http://www.telos.de). It was
*  converted from C++ to C, and modified to add control over the recording level.
*  Converted to fixed point by Phil Frisbie, Jr. 4/12/2003
*/

HL_EXP hvdi_agc* HL_APIENTRY hvdiNewAGC(float level)
{
    hvdi_agc *agc = malloc(sizeof(hvdi_agc));

    if(agc == NULL)
    {
        return NULL;
    }
    agc->sample_max = 1;
    agc->counter = 0;
    agc->igain = 65536;
    if(level > 1.0f)
    {
        level = 1.0f;
    }
    else if(level < 0.5f)
    {
        level = 0.5f;
    }
    agc->ipeak = (int)(SHRT_MAX * level * 65536);
    agc->silence_counter = 0;
    return agc;
}

HL_EXP void HL_APIENTRY hvdiAGC(hvdi_agc *agc, short *buffer, int len)
{
    int i;

    for(i=0;i<len;i++)
    {
        long gain_new;
        int sample;

        /* get the abs of buffer[i] */
        sample = buffer[i];
        sample = (sample < 0 ? -(sample):sample);

        if(sample > (int)agc->sample_max)
        {
            /* update the max */
            agc->sample_max = (unsigned int)sample;
        }
        agc->counter ++;

        /* Will we get an overflow with the current gain factor? */
        if (((sample * agc->igain) >> 16) > agc->ipeak)
        {
            /* Yes: Calculate new gain. */
            agc->igain = ((agc->ipeak / agc->sample_max) * 62259) >> 16;
            agc->silence_counter = 0;
            buffer[i] = (short) ((buffer[i] * agc->igain) >> 16);
            continue;
        }

        /* Calculate new gain factor 10x per second */
        if (agc->counter >= 8000/10) 
        {
            if (agc->sample_max > 800)        /* speaking? */
            {
                gain_new = ((agc->ipeak / agc->sample_max) * 62259) >> 16;
                
                if (agc->silence_counter > 40)  /* pause -> speaking */
                    agc->igain += (gain_new - agc->igain) >> 2;
                else
                    agc->igain += (gain_new - agc->igain) / 20;

                agc->silence_counter = 0;
            }
            else   /* silence */
            {
                agc->silence_counter++;
                /* silence > 2 seconds: reduce gain */
                if ((agc->igain > 65536) && (agc->silence_counter >= 20))
                    agc->igain = (agc->igain * 62259) >> 16;
            }
            agc->counter = 0;
            agc->sample_max = 1;
        }
        buffer[i] = (short) ((buffer[i] * agc->igain) >> 16);
    }
}

HL_EXP void HL_APIENTRY hvdiDeleteAGC(hvdi_agc *agc)
{
    free(agc);
}

HL_EXP void HL_APIENTRY hvdiMix(short *outbuf, short **inbuf, int number, int inlen)
{
    int i, j;

    for(i=0;i<inlen;i++)
    {
        long    total = 0;

        for(j=0;j<number;j++)
        {
            total += *(inbuf[j] + i);
        }
        if(total > SHRT_MAX)
        {
            *outbuf++ = (short)SHRT_MAX;
        }
        else if(total < SHRT_MIN)
        {
            *outbuf++ = (short)SHRT_MIN;
        }
        else
        {
            *outbuf++ = (short)total;
        }
    }
}

HL_EXP void HL_APIENTRY hvdiHint(int name, int arg)
{
    switch (name) {
    case HVDI_NORMAL:
        hvdi_state.celp_codebook = 256;
        hvdi_state.gsm_lpt = 0;
        hvdi_state.celp_fast_gain = 0;
        break;

    case HVDI_FAST:
        hvdi_state.celp_codebook = 128;
        hvdi_state.gsm_lpt = 1;
        hvdi_state.celp_fast_gain = 1;
        break;

    case HVDI_FASTEST:
        hvdi_state.celp_codebook = 32;
        hvdi_state.gsm_lpt = 1;
        hvdi_state.celp_fast_gain = 1;
        break;

    case HVDI_CELP_CODEBOOK:
        hvdi_state.celp_codebook = min(max(arg, 32), 256);
        break;

    case HVDI_SEQUENCE:
        hvdi_state.sequence = arg;
        break;

    case HVDI_AUTO_VOX:
        hvdi_state.autoVOX = arg;
        break;

    case HVDI_VOX_LEVEL:
        hvdi_state.VOXlevel = (int) exp(log(32767.0) * ((1000 - min(max(arg, 0), 1000)) / 1000.0));;
        break;

    case HVDI_VOX_SPEED:
        hvdi_state.VOXspeed = arg;
        break;

    case HVDI_COMFORT_NOISE:
        hvdi_state.comfortnoise = arg;
        break;

    case HVDI_NOISE_LEVEL:
        hvdi_state.noiselevel = (int) exp(log(32767.0) * ((min(max(arg, 0), 1000) + 1000 ) / 2000.0));
        break;

    }
}

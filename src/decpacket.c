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

/* 16 bit signed PN generator by Phil Frisbie */
/* only generates 32K numbers before looping */
HL_INLINE int random16(void)
{
    static int the_random = 0xffff;
    static int temp;

    temp = the_random & 0x6;
    if(temp == 0 || temp == 0x6)
    {
        the_random >>= 1;
    }
    else
    {
        the_random >>= 1;
        the_random |= 0x8000;
    }
    return(the_random-32767);
}

static int openlpcdecode(short *out, unsigned char *in, int buflen, int paclen, void *state, int framesize)
{
    if(buflen >= (paclen / OPENLPC_ENCODED_FRAME_SIZE * framesize))
    {
        openlpc_decoder_state *st = (openlpc_decoder_state *)((hvdi_dec_state *)state)->state;
        int         len = 0;
        int         i;
        
        /* decode the sound*/
        for(i=0;i<paclen;i+=OPENLPC_ENCODED_FRAME_SIZE)
        {
            len += openlpc_decode(&in[i], &out[len], st);
        }
        return len;
    }
    return 0;
}

static int lpc10decode(short *out, unsigned char *in, int buflen, int paclen, void *state)
{
    if(buflen >= (paclen / LPC10_ENCODED_FRAME_SIZE * LPC10_SAMPLES_PER_FRAME))
    {
        lpc10_decoder_state *st = (lpc10_decoder_state *)((hvdi_dec_state *)state)->state;
        int         len = 0;
        int         i;
        
        /* decode the sound*/
        for(i=0;i<paclen;i+=LPC10_ENCODED_FRAME_SIZE)
        {
            len += lpc10_decode(&in[i], &out[len], st);
        }
        return len;
    }
    return 0;
}

static int lpcdecode(short *out, unsigned char *in, int buflen, int paclen, void *state)
{
    if(buflen >= (paclen / LPC_ENCODED_FRAME_SIZE * LPC_SAMPLES_PER_FRAME))
    {
        lpc_decoder_state *st = (lpc_decoder_state *)((hvdi_dec_state *)state)->state;
        int         len = 0;
        int         i;
        
        /* decode the sound*/
        for(i=0;i<paclen;i+=LPC_ENCODED_FRAME_SIZE)
        {
            len += lpc_decode(&in[i], &out[len], st);
        }
        return len;
    }
    return 0;
}

static int gsmdecode(short *out, unsigned char *in, int buflen, int paclen, void *state)
{
    if(buflen >= (paclen / GSM_ENCODED_FRAME_SIZE * GSM_SAMPLES_PER_FRAME))
    {
        struct gsm_state *st = (struct gsm_state *)((hvdi_dec_state *)state)->state;
        int         len = 0;
        int         i;
        
        /* decode the sound*/
        for(i=0;i<paclen;i+=GSM_ENCODED_FRAME_SIZE)
        {
            len += gsm_decode(st, &in[i], &out[len]);
        }
        return len;
    }
    return 0;
}

static int adpcmdecode(short *out, unsigned char *in, int buflen, int paclen)
{
    static struct adpcm_state  st = {0,(char)0};
    int                 count = 0;
    
    if(buflen >= (int)((paclen - sizeof(st.valprev) - sizeof(st.index)) * 2))
    {
        /* read in the codec state */
        readSignedShort(in, count, st.valprev);
        readSignedByte(in, count, st.index);

        paclen -= count;
        return adpcm_decoder(&in[count], out, paclen, &st);
    }
    return 0;
}

static int ulawdecode(short *out, unsigned char *in, int buflen, int paclen)
{
    if(buflen >= paclen)
    {
        return ulawDecode(in, out, paclen);
    }
    return 0;
}

static int celpdecode(short *out, unsigned char *in, int buflen, int paclen, void *state, int framesize)
{
    if(buflen >= (paclen / CELP_ENCODED_FRAME_SIZE * framesize))
    {
        celp_decoder_state *st = (celp_decoder_state *)((hvdi_dec_state *)state)->state;
        int         len = 0;
        int         i;
        
        /* decode the sound*/
        for(i=0;i<paclen;i+=CELP_ENCODED_FRAME_SIZE)
        {
            len += celp_decode(&in[i], &out[len], st);
        }
        return len;
    }
    return 0;
}

static int vbrlpc10decode(short *out, unsigned char *in, int buflen, int paclen, void *state)
{
    if(buflen >= (paclen / LPC10_ENCODED_FRAME_SIZE * LPC10_SAMPLES_PER_FRAME))
    {
        lpc10_decoder_state *st = (lpc10_decoder_state *)((hvdi_dec_state *)state)->state;
        int         len = 0;
        int         i, p = 0;
        
        /* decode the sound*/
        for(i=0;i<paclen;i+=p)
        {
            /* we need to check again for out buffer space since the check */
            /* above does not take into account the shorter VBR frames */
            if((buflen - len) < LPC10_SAMPLES_PER_FRAME)
            {
                return 0;
            }
            len += vbr_lpc10_decode(&in[i], &out[len], st, &p);
        }
        return len;
    }
    return 0;
}

static int silencedecode(short *out, unsigned char *in, int buflen, int paclen)
{
    int count = 0;
    int len;

    if(paclen < 2)
    {
        return 0;
    }
    readSignedShort(in, count, len);
    if(buflen >= len)
    {
        if(hvdi_state.comfortnoise != 0)
        {
            /* fill buffer with comfort noise */
            int i = (int)len;
            int level = hvdi_state.noiselevel;

            while(i-- > 0)
            {
                *out = (short)((random16() * level) >> 15);
                out++;
            }
        }
        else
        {
            /* zero out buffer for perfect silence */
            memset(out, 0, (size_t)(len * 2));
        }
        return len;
    }
    return 0;
}

HL_EXP int HL_APIENTRY hvdiPacketDecode(unsigned char *packet, int paclen, short *buffer,
                                        int buflen, hcrypt_key *key, hvdi_dec_state *state)
{
    int             len = 0;
    int             count = 0;
    unsigned char   *data;
    unsigned char   codec;
    unsigned short  sequence;

    /* safety check before we try to read from the packet */
    if(paclen < 3)
    {
        return NL_FALSE;
    }
    /* read the packet header */
    readByte(packet, count, codec);
    
    /* authenticate the packet if needed */
    if(key != NULL)
    {
        if((codec&HVDI_ENCRYPT_BIT) == (unsigned char)0)
        {
            /* We have a problem! The packet is not labled as encrypted, */
            /* but the calling app supplied a key. To be safe, we will */
            /* drop the packet right now! */
            return NL_FALSE;
        }
        if(hcryptAuthenticatePacket(packet, paclen, key) == 0)
        {
            return NL_FALSE;
        }
        paclen -= 4;
    }

    /* check for sequence */
    if((codec&HVDI_SEQUENCE_BIT) != (unsigned char)0)
    {
        readShort(packet, count, sequence);
        
        /* check the sequence */
        if(sequence < state->sequence)
        {
            /* check for wrapping sequence number */
            if(state->sequence - sequence > 3000)
            {
                /* check for dropped packets */
                if(!(state->sequence == 65536 && sequence == 0))
                {
                    /* we have dropped one or more packets, so reset the codec*/
                    hvdiResetDecoderCodec(state);
                }
                state->sequence = 0;
            }
            /* this is a delayed packet, so drop it */
            return 0;
        }
        state->sequence++;
        if(state->sequence < sequence)
        {
            /* we have dropped one or more packets, so reset the codec*/
            hvdiResetDecoderCodec(state);
        }
        state->sequence = sequence;
    }
    
    /* data points to the start of the encoded speech */
    data = &packet[count];
    paclen -= count;
    
    /* remove HVDI_ENCRYPT_BIT and HVDI_SEQUENCE_BIT */
    codec &= 0x3F;

    /* decrypt the packet if needed */
    if(key != NULL && codec != (unsigned char)HV_SILENCE_CODEC)
    {
        hcryptDecryptPacket(data, data, paclen, key);
    }

    /* check if codec has changed from the last packet */
    /* but only change if it is NOT silence codec */
    if(codec != state->codec && codec != (unsigned char)HV_SILENCE_CODEC)
    {
        if(hvdiSetDecoderCodec(codec, state) == NL_FALSE)
            return NL_FALSE;
    }
    
    /* decode the speech */
    switch (codec) {
    case HV_1_4K_CODEC:
        len = openlpcdecode(buffer, data, buflen, paclen, state, OPENLPC_FRAMESIZE_1_4);
        break;
        
    case HV_1_8K_CODEC:
        len = openlpcdecode(buffer, data, buflen, paclen, state, OPENLPC_FRAMESIZE_1_8);
        break;
        
    case HV_2_4K_CODEC:
        len = lpc10decode(buffer, data, buflen, paclen, state);
        break;
        
    case HV_4_8K_CODEC:
        len = lpcdecode(buffer, data, buflen, paclen, state);
        break;
        
    case HV_13_2K_CODEC:
        len = gsmdecode(buffer, data, buflen, paclen, state);
        break;
        
    case HV_32K_CODEC:
        len = adpcmdecode(buffer, data, buflen, paclen);
        break;
        
    case HV_64K_CODEC:
        len = ulawdecode(buffer, data, buflen, paclen);
        break;
        
    case HV_4_5K_CODEC:
        len = celpdecode(buffer, data, buflen, paclen, state, CELP_4_5_FRAMESIZE);
        break;
        
    case HV_3_0K_CODEC:
        len = celpdecode(buffer, data, buflen, paclen, state, CELP_3_0_FRAMESIZE);
        break;
        
    case HV_2_3K_CODEC:
        len = celpdecode(buffer, data, buflen, paclen, state, CELP_2_3_FRAMESIZE);
        break;
        
    case HV_VBR_2_4K_CODEC:
        len = vbrlpc10decode(buffer, data, buflen, paclen, state);
        break;
        
    case HV_SILENCE_CODEC:
        len = silencedecode(buffer, data, buflen, paclen);
        break;

    default:
        /* invalid codec */
        return NL_FALSE;
    }
    
    return len;
}



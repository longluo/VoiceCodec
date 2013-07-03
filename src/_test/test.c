/*
  Packet test program for the HawkVoiceDI cross platform network library
  Copyright (C) 2001-2004 Phil Frisbie, Jr. (phil@hawksoft.com)
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../hvdi.h"
#include "../ulaw/u-law.h"

/* This was copied from nl.h so that it did not need to be included */

/* max packet size for NL_UNRELIABLE and NL_RELIABLE_PACKETS */
#define NL_MAX_PACKET_LENGTH   16384

#if defined WIN32 || defined WIN64 || defined __i386__ || defined __alpha__
#define NL_LITTLE_ENDIAN
#endif

#ifdef NL_LITTLE_ENDIAN
#define nlSwaps(x) (unsigned short)(((((unsigned short)x) & 0x00ff) << 8) |\
                                ((((unsigned short)x) & 0xff00) >> 8))

#define nlSwapl(x) (unsigned long)(((((unsigned long)x) & 0x000000ff) << 24) | \
                                ((((unsigned long)x) & 0x0000ff00) << 8) | \
                                ((((unsigned long)x) & 0x00ff0000) >> 8) | \
                                ((((unsigned long)x) & 0xff000000) >> 24))

#else
/* no conversion needed for big endian */
#define nlSwaps(x) (x)
#define nlSwapl(x) (x)
#endif /* NL_LITTLE_ENDIAN */

#if defined WIN32 || defined WIN64

#include <io.h>
#define open	    _open
#define close	    _close
#define read	    _read
#define write       _write
#define lseek       _lseek
#define O_RDONLY    _O_RDONLY
#define O_RDWR      _O_RDWR
#define O_BINARY    _O_BINARY
#define O_CREAT     _O_CREAT
#define O_TRUNC     _O_TRUNC
#define S_IREAD     _S_IREAD
#define S_IWRITE    _S_IWRITE

#else

#include <sys/io.h>
#define O_BINARY 0

#endif

/* this is the LCM of 160, 180, 240, 320, 360, and 480 to simplify the code */
#define NUM_SAMPLES     2880

#define AU_U_LAW            1
#define AU_PCM_16_BIT       3
#define AU_NOT_SUPPORTED    0


struct auheader {
long magic, hsize, dsize, emode, rate, nchan;
};

int filetype;

static void writeHeader(int f)
{
    struct auheader h;

    h.magic = nlSwapl(0x2e736e64);
    h.hsize = nlSwapl(sizeof(struct auheader));
    h.dsize = nlSwapl(0xffffffff);
    h.emode = nlSwapl(AU_U_LAW);
    h.rate = nlSwapl(8000);
    h.nchan = nlSwapl(1);

    write(f, &h, sizeof(h));
}

static int readHeader(int f)
{
    struct auheader h;

    read(f, &h, sizeof(h));

    h.emode = nlSwapl(h.emode);
    if(h.emode == AU_U_LAW || h.emode == AU_PCM_16_BIT)
    {
        return h.emode;
    }
    return AU_NOT_SUPPORTED;
}

void swapSamples(short *buffer, int len)
{
    int i;

    for(i=0;i<len;i++)
    {
        buffer[i] = nlSwaps(buffer[i]);
    }
}

static int readSamples(int f, int type, short *buffer, int len)
{
    int count;

    if(type == AU_PCM_16_BIT)
    {
        /* no conversion needed, just swap */
        count =  read(f, buffer, sizeof(short) * len);
        count /= 2;
        swapSamples(buffer, count);

        return count;
    }
    else
    {
        unsigned char temp[NUM_SAMPLES];

        /* convert the u-law samples to 16 bit */
        count = read(f, temp, len);
        ulawDecode(temp, buffer, count);
        return count;
    }
}

void writeSamples(int f, short *buffer, int len)
{
    unsigned char temp[NUM_SAMPLES * 3];

    ulawEncode(buffer, temp, len);

    (void)write(f, temp, len);
}

int main(int argc, char **argv)
{
    int             infile, ulawfile, adpcmfile, gsmfile, lpcfile, lpc10file, type;
    int             lpc14file, lpc18file, celp45file, celp30file, celp23file, vbrlpc10file;
    hvdi_enc_state  *ulawenc, *adpcmenc, *gsmenc, *lpcenc, *lpc10enc, *lpc14enc, *lpc18enc,
                    *celp45enc, *celp30enc, *celp23enc, *vbrlpc10enc;
    hvdi_dec_state  *ulawdec, *adpcmdec, *gsmdec, *lpcdec, *lpc10dec, *lpc14dec, *lpc18dec,
                    *celp45dec, *celp30dec, *celp23dec, *vbrlpc10dec;
    int             buflen = NUM_SAMPLES;
    hcrypt_key      *key = NULL;
    hcrypt_salt     *salt;
    char            keystring[] = "HawkVoice ROCKS!!!";
    hvdi_agc        *agc;
    hvdi_vox        *vox;
    int             passed = 0, nopass = 0;

    /* the salt would need to be passed from the other clients */
    salt = hcryptNewSalt();
    key = hcryptNewKey(keystring, salt);
    /* open the input file */
    if(argc > 1)
    {
        infile = open(argv[1], O_BINARY|O_RDONLY);
    }
    else
    {
        infile = open("16bit.au", O_BINARY|O_RDONLY);
    }
    if(infile < 0)
    {
        printf("Could not open input file\n");
        return (1);
    }
    type = readHeader(infile);

    if(type == AU_NOT_SUPPORTED)
    {
        close(infile);
        return 1;
    }

    /* set the encoding preferences */
    hvdiHint(HVDI_NORMAL, 0);

    /* open the output files and create state objects */
    ulawfile = open("ulawx.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(ulawfile);
    ulawenc = hvdiNewEncState();
    ulawdec = hvdiNewDecState();
    hvdiEncStateSetCodec(ulawenc, HV_ULAW_CODEC);

    adpcmfile = open("adpcmx.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(adpcmfile);
    adpcmenc = hvdiNewEncState();
    adpcmdec = hvdiNewDecState();
    hvdiEncStateSetCodec(adpcmenc, HV_ADPCM_32_CODEC);

    gsmfile = open("gsmx.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(gsmfile);
    gsmenc = hvdiNewEncState();
    gsmdec = hvdiNewDecState();
    hvdiEncStateSetCodec(gsmenc, HV_GSM_CODEC);

    lpcfile = open("lpcx.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(lpcfile);
    lpcenc = hvdiNewEncState();
    lpcdec = hvdiNewDecState();
    hvdiEncStateSetCodec(lpcenc, HV_LPC_CODEC);

    lpc10file = open("lpc10x.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(lpc10file);
    lpc10enc = hvdiNewEncState();
    lpc10dec = hvdiNewDecState();
    hvdiEncStateSetCodec(lpc10enc, HV_LPC10_CODEC);

    lpc14file = open("lpc14x.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(lpc14file);
    lpc14enc = hvdiNewEncState();
    lpc14dec = hvdiNewDecState();
    hvdiEncStateSetCodec(lpc14enc, HV_LPC_1_4_CODEC);

    lpc18file = open("lpc18x.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(lpc18file);
    lpc18enc = hvdiNewEncState();
    lpc18dec = hvdiNewDecState();
    hvdiEncStateSetCodec(lpc18enc, HV_LPC_1_8_CODEC);

    celp45file = open("celp45x.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(celp45file);
    celp45enc = hvdiNewEncState();
    celp45dec = hvdiNewDecState();
    hvdiEncStateSetCodec(celp45enc, HV_CELP_4_5_CODEC);

    celp30file = open("celp30x.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(celp30file);
    celp30enc = hvdiNewEncState();
    celp30dec = hvdiNewDecState();
    hvdiEncStateSetCodec(celp30enc, HV_CELP_3_0_CODEC);

    celp23file = open("celp23x.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(celp23file);
    celp23enc = hvdiNewEncState();
    celp23dec = hvdiNewDecState();
    hvdiEncStateSetCodec(celp23enc, HV_CELP_2_3_CODEC);

    vbrlpc10file = open("vbrlpc10x.au", O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(vbrlpc10file);
    vbrlpc10enc = hvdiNewEncState();
    vbrlpc10dec = hvdiNewDecState();
    hvdiEncStateSetCodec(vbrlpc10enc, HV_VBR_LPC10_CODEC);

    agc = hvdiNewAGC(0.9f);
    vox = hvdiNewVOX(HVDI_VOX_FAST, 600);

    /* hints go here */
    hvdiHint(HVDI_FASTEST, 0);
    hvdiHint(HVDI_SEQUENCE, 1);
    hvdiHint(HVDI_AUTO_VOX, 1);
    hvdiHint(HVDI_VOX_LEVEL, 300);
    hvdiHint(HVDI_COMFORT_NOISE, 1);
    hvdiHint(HVDI_NOISE_LEVEL, 100);

    while(buflen == NUM_SAMPLES)
    {
        int             encodedlen, decodedlen;
        short           samples[NUM_SAMPLES];
        short           decoded[NUM_SAMPLES * 2];
        unsigned char   packet[NL_MAX_PACKET_LENGTH];
        int             paclen = NL_MAX_PACKET_LENGTH;
        int             valid;

        buflen = readSamples(infile, type, samples, NUM_SAMPLES);

        /* exercise the AGC and VOX first */
        hvdiAGC(agc, samples, buflen);
        if(hvdiVOX(vox, samples, buflen) == 1)
        {
            passed++;
        }
        else
        {
            nopass++;
        }

        /* u-law */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, ulawenc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, ulawdec);
        writeSamples(ulawfile, decoded, decodedlen);

        /* ADPCM */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, adpcmenc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, adpcmdec);
        writeSamples(adpcmfile, decoded, decodedlen);

        /* GSM */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, gsmenc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, gsmdec);
        writeSamples(gsmfile, decoded, decodedlen);

        /* LPC */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, lpcenc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, lpcdec);
        writeSamples(lpcfile, decoded, decodedlen);

        /* LPC-10 */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, lpc10enc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, lpc10dec);
        writeSamples(lpc10file, decoded, decodedlen);

        /* OpenLPC 1.4K */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, lpc14enc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, lpc14dec);
        writeSamples(lpc14file, decoded, decodedlen);

        /* OpenLPC 1.8K */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, lpc18enc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, lpc18dec);
        writeSamples(lpc18file, decoded, decodedlen);

        /* CELP 4.5K */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, celp45enc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, celp45dec);
        writeSamples(celp45file, decoded, decodedlen);

        /* CELP 3.0K */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, celp30enc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, celp30dec);
        writeSamples(celp30file, decoded, decodedlen);

        /* CELP 2.3K */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, celp23enc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, celp23dec);
        writeSamples(celp23file, decoded, decodedlen);

        /* VBR-LPC-10 */
        encodedlen = hvdiPacketEncode(packet, buflen, samples, paclen, key, vbrlpc10enc);
        valid = hvdiPacketIsVoice(packet, encodedlen);
        decodedlen = hvdiPacketDecode(packet, encodedlen, decoded, NUM_SAMPLES * 2, key, vbrlpc10dec);
        writeSamples(vbrlpc10file, decoded, decodedlen);

    }

    printf("%d samples passed the VOX, and %d did not pass\n", passed, nopass);
    hvdiDeleteEncState(ulawenc);
    hvdiDeleteDecState(ulawdec);
    hvdiDeleteEncState(adpcmenc);
    hvdiDeleteDecState(adpcmdec);
    hvdiDeleteEncState(gsmenc);
    hvdiDeleteDecState(gsmdec);
    hvdiDeleteEncState(lpcenc);
    hvdiDeleteDecState(lpcdec);
    hvdiDeleteEncState(lpc10enc);
    hvdiDeleteDecState(lpc10dec);
    hvdiDeleteEncState(lpc14enc);
    hvdiDeleteDecState(lpc14dec);
    hvdiDeleteEncState(lpc18enc);
    hvdiDeleteDecState(lpc18dec);
    hvdiDeleteEncState(celp45enc);
    hvdiDeleteDecState(celp45dec);
    hvdiDeleteEncState(celp30enc);
    hvdiDeleteDecState(celp30dec);
    hvdiDeleteEncState(celp23enc);
    hvdiDeleteDecState(celp23dec);
    hvdiDeleteEncState(vbrlpc10enc);
    hvdiDeleteDecState(vbrlpc10dec);

    hcryptDeleteKey(key);
    hcryptDeleteSalt(salt);
    hvdiDeleteAGC(agc);
    hvdiDeleteVOX(vox);

    close(lpc10file);
    close(lpc14file);
    close(lpc18file);
    close(lpcfile);
    close(gsmfile);
    close(adpcmfile);
    close(ulawfile);
    close(celp45file);
    close(celp30file);
    close(celp23file);
    close(vbrlpc10file);
    close(infile);
    return 0;
}


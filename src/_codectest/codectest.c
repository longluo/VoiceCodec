/*
  Codec test program for the HawkVoice cross platform voice library
  Copyright (C) 2000-2004 Phil Frisbie, Jr. (phil@hawksoft.com)
*/
/*
  To test UNICODE on Windows NT/2000/XP, uncomment both the defines below and compile
  this program.
*/
//#define _UNICODE
//#define UNICODE

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../hawklib.h"
#include "../ulaw/u-law.h"
#include "../adpcm/adpcm.h"
#include "../gsm/gsm.h"
#include "../lpc/lpc.h"
#include "../lpc10/lpc10.h"
#include "../openlpc/openlpc.h"
#include "../celp/celp.h"

#ifdef HL_WINDOWS_APP
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#endif

#ifndef _INC_TCHAR
#ifdef _UNICODE
#define TEXT(x)    L##x
#define _TCHAR      wchar_t
#define _tmain      wmain
#define _tprintf    wprintf
#define _stprintf   swprintf
#define _tcslen     wcslen
#ifdef HL_WINDOWS_APP
#define _ttoi       _wtoi
#else /* !HL_WINDOWS_APP*/
#define _ttoi       wtoi
#endif /* !HL_WINDOWS_APP*/
#else /* !UNICODE */
#define TEXT(x)    x
#define _TCHAR      char
#define _tmain      main
#define _tprintf    printf
#define _stprintf   sprintf
#define _tcslen     strlen
#endif /* !UNICODE */
#endif /* _INC_TCHAR */

/* This was copied from nl.h so that it did not need to be included */

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

#ifdef HL_WINDOWS_APP
#include <io.h>
#include <windows.h>
#ifdef _UNICODE
#define _topen	    _wopen
#else
#define _topen	    _open
#endif /* !_UNICODE */
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
#define _topen	    open
#define O_BINARY 0

#endif /* !HL_WINDOWS_APP */

#if defined _MSC_VER && defined _M_IX86 

#define RDTSC __asm _emit 0x0f __asm _emit 0x31

unsigned long getcycles(void)
{
	unsigned long	cycles;

__asm{
		RDTSC				; get the count
		mov cycles, eax		; copy register
	}
	return cycles;
}

#elif defined __GNUC__ && defined __i386__

unsigned long getcycles(void)
{
	unsigned long	cycles;

    asm("rdtsc" : "=a" (cycles));
	return cycles;
}

#else

unsigned long getcycles(void)
{
    return 0;
}

#endif /* !_MSC_VER && _M_IX86*/

unsigned long countcycles(unsigned long a)
{
    unsigned long b = getcycles();

    if(a == 0 && b == 0)
        return 0;

    if(a < b)
    {
        return (b - a);
    }
    else
    {
        return ((unsigned long)0xffffffff - a + b);
    }
}


#define AU_U_LAW            1
#define AU_PCM_8_BIT        2
#define AU_PCM_16_BIT       3
#define AU_NOT_SUPPORTED    0

/* number of samples to process at a time, ALWAYS an even number */
#define NUM_SAMPLES     160
#define LPC10_SAMPLES   180

#define CELP_CODEBOOK   32
#define CELP_FAST       1

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

    h.rate = nlSwapl(h.rate);
    h.nchan = nlSwapl(h.nchan);
    if(h.rate > 9000 || h.nchan != 1)
    {
        return AU_NOT_SUPPORTED;
    }
    h.emode = nlSwapl(h.emode);
    if(h.emode == AU_U_LAW || h.emode == AU_PCM_16_BIT || h.emode == AU_PCM_8_BIT)
    {
        return h.emode;
    }
    return AU_NOT_SUPPORTED;
}

static void swapSamples(short *buffer, int len)
{
    int i;

    for(i=0;i<len;i++)
    {
        buffer[i] = nlSwaps(buffer[i]);
    }
}

static void expandSamples(unsigned char *in, short *buffer, int len)
{
    int i;

    for(i=0;i<len;i++)
    {
        buffer[i] = in[i]|(in[i]<<8);
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
    else if(type == AU_PCM_8_BIT)
    {
        unsigned char temp[1000];

        count =  read(f, buffer, len);
        expandSamples(temp, buffer, count);

        return count;
    }
    else
    {
        unsigned char temp[1000];

        /* convert the u-law samples to 16 bit */
        count = read(f, temp, len);
        ulawDecode(temp, buffer, count);
        return count;
    }
}

static void writeSamples(int f, short *buffer, int len)
{
    unsigned char temp[1000];

    ulawEncode(buffer, temp, len);

    (void)write(f, temp, len);
}
#ifdef HL_WINDOWS_APP
int __cdecl _tmain(int argc, _TCHAR **argv)
#else
int _tmain(int argc, _TCHAR **argv)
#endif
{
    int                         infile, ulawfile, adpcmfile, gsmfile, lpcfile, lpc10file, type;
    int                         lpc18file, lpc14file, celp45file, celp30file, celp23file, vbrlpc10file;
    int                         len = NUM_SAMPLES;
    struct adpcm_state          adpcmencode = {0, 0};
    struct adpcm_state          adpcmdecode = {0, 0};
    struct gsm_state            *gsmencode;
    struct gsm_state            *gsmdecode;
    lpc_encoder_state           *lpcencode;
    lpc_decoder_state           *lpcdecode;
    lpc10_encoder_state         *lpc10encode;
    lpc10_decoder_state         *lpc10decode;
    openlpc_encoder_state       *lpc18encode;
    openlpc_decoder_state       *lpc18decode;
    openlpc_encoder_state       *lpc14encode;
    openlpc_decoder_state       *lpc14decode;
    celp_encoder_state          *celpencode;
    celp_decoder_state          *celpdecode;
    lpc10_encoder_state         *vbrlpc10encode;
    lpc10_decoder_state         *vbrlpc10decode;
    unsigned long               ulawe, adpcme, gsme, lpce, lpc10e, lpc18e, lpc14e, vbrlpc10e, count;
    unsigned long               ulawd, adpcmd, gsmd, lpcd, lpc10d, lpc18d, lpc14d, celpe, celpd, vbrlpc10d;
    int                         mytrue = 1;
#ifdef WIN32
	int tp, pp;
	HANDLE t, p;

	t = GetCurrentThread();
	tp = GetThreadPriority(t);
	SetThreadPriority(t, THREAD_PRIORITY_TIME_CRITICAL);
	p = GetCurrentProcess();
	pp = GetPriorityClass(p);
	SetPriorityClass(p, HIGH_PRIORITY_CLASS);
#endif

    /* open the input file */
    if(argc > 1)
    {
        infile = _topen(argv[1], O_BINARY|O_RDONLY);
    }
    else
    {
        infile = _topen(TEXT("16bit.au"), O_BINARY|O_RDONLY);
    }
    if(infile < 0)
    {
        _tprintf(TEXT("Could not open input file\n"));
        return (1);
    }
    type = readHeader(infile);

    /* open the output files */
    ulawfile = _topen(TEXT("ulaw.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(ulawfile);

    adpcmfile = _topen(TEXT("adpcm.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(adpcmfile);

    gsmfile = _topen(TEXT("gsm.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(gsmfile);
    gsmencode = gsm_create();
    gsmdecode = gsm_create();
    (void)gsm_option(gsmencode, GSM_OPT_LTP_CUT, &mytrue);

    lpcfile = _topen(TEXT("lpc.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(lpcfile);
    lpcencode = create_lpc_encoder_state();
    init_lpc_encoder_state(lpcencode);
    lpcdecode = create_lpc_decoder_state();
    init_lpc_decoder_state(lpcdecode);

    lpc10file = _topen(TEXT("lpc10.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(lpc10file);
    lpc10encode = create_lpc10_encoder_state();
    init_lpc10_encoder_state(lpc10encode);
    lpc10decode = create_lpc10_decoder_state();
    init_lpc10_decoder_state(lpc10decode);

    lpc18file = _topen(TEXT("lpc18.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(lpc18file);
    lpc18encode = create_openlpc_encoder_state();
    init_openlpc_encoder_state(lpc18encode, OPENLPC_FRAMESIZE_1_8);
    lpc18decode = create_openlpc_decoder_state();
    init_openlpc_decoder_state(lpc18decode, OPENLPC_FRAMESIZE_1_8);

    lpc14file = _topen(TEXT("lpc14.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(lpc14file);
    lpc14encode = create_openlpc_encoder_state();
    init_openlpc_encoder_state(lpc14encode, OPENLPC_FRAMESIZE_1_4);
    lpc14decode = create_openlpc_decoder_state();
    init_openlpc_decoder_state(lpc14decode, OPENLPC_FRAMESIZE_1_4);

    vbrlpc10file = _topen(TEXT("vbrlpc10.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(vbrlpc10file);
    vbrlpc10encode = create_lpc10_encoder_state();
    init_lpc10_encoder_state(vbrlpc10encode);
    vbrlpc10decode = create_lpc10_decoder_state();
    init_lpc10_decoder_state(vbrlpc10decode);

    ulawe = adpcme = gsme = lpce = lpc10e = lpc18e = lpc14e = celpe = vbrlpc10e = count = 0;
    ulawd = adpcmd = gsmd = lpcd = lpc10d = lpc18d = lpc14d = celpd = vbrlpc10d = 0;
#if 1
    while(len == NUM_SAMPLES)
    {
        int             encodedlen;
        short           samples[NUM_SAMPLES];
        short           decoded[NUM_SAMPLES];
        unsigned char   encoded[NUM_SAMPLES];
        unsigned long   c;

        len = readSamples(infile, type, samples, NUM_SAMPLES);
        count++;
        /* u-law */
        c = getcycles();
        encodedlen = ulawEncode(samples, encoded, len);
        ulawe += countcycles(c);
        c = getcycles();
        (void)ulawDecode(encoded, decoded, encodedlen);
        ulawd += countcycles(c);
        writeSamples(ulawfile, decoded, len);

        /* ADPCM */
        c = getcycles();
        encodedlen = adpcm_coder(samples, encoded, len, &adpcmencode);
        adpcme += countcycles(c);
        c = getcycles();
        (void)adpcm_decoder(encoded, decoded, encodedlen, &adpcmdecode);
        adpcmd += countcycles(c);
        writeSamples(adpcmfile, decoded, len);

        /* GSM */
        c = getcycles();
        encodedlen = gsm_encode(gsmencode, samples, encoded);
        gsme += countcycles(c);
        c = getcycles();
        (void)gsm_decode(gsmdecode, encoded, decoded);
        gsmd += countcycles(c);
        writeSamples(gsmfile, decoded, len);

        /* LPC */
        c = getcycles();
        encodedlen = lpc_encode(samples, encoded, lpcencode);
        lpce += countcycles(c);
        c = getcycles();
        (void)lpc_decode(encoded, decoded, lpcdecode);
        lpcd += countcycles(c);
        writeSamples(lpcfile, decoded, len);
    }
    count /= 8000/NUM_SAMPLES;
    _tprintf(TEXT("u-law cycles per second: encode = %lu, decode = %lu\n"), ulawe / count, ulawd / count);
    _tprintf(TEXT("ADPCM cycles per second: encode = %lu, decode = %lu\n"), adpcme / count, adpcmd / count);
    _tprintf(TEXT("GSM cycles per second: encode = %lu, decode = %lu\n"), gsme / count, gsmd / count);
    _tprintf(TEXT("LPC cycles per second: encode = %lu, decode = %lu\n"), lpce / count, lpcd / count);
#endif
#if 1
    /* reset the input file for the LPC-10 codec */
    lseek(infile, 0, SEEK_SET);
    type = readHeader(infile);
    len = LPC10_SAMPLES;
    count = 0;

    while(len == LPC10_SAMPLES)
    {
        int             encodedlen, p;
        short           samples[LPC10_SAMPLES];
        short           decoded[LPC10_SAMPLES];
        unsigned char   encoded[LPC10_SAMPLES];
        unsigned long   c;

        len = readSamples(infile, type, samples, LPC10_SAMPLES);
        count++;

        /* LPC-10 */
        c = getcycles();
        encodedlen = lpc10_encode(samples, encoded, lpc10encode);
        lpc10e += countcycles(c);
        c = getcycles();
        (void)lpc10_decode(encoded, decoded, lpc10decode);
        lpc10d += countcycles(c);
        writeSamples(lpc10file, decoded, len);

        /* VBR-LPC-10 */
        c = getcycles();
        encodedlen = vbr_lpc10_encode(samples, encoded, vbrlpc10encode);
        vbrlpc10e += countcycles(c);
        c = getcycles();
        (void)vbr_lpc10_decode(encoded, decoded, vbrlpc10decode, &p);
        vbrlpc10d += countcycles(c);
        writeSamples(vbrlpc10file, decoded, len);
    }
    count /= 8000/LPC10_SAMPLES;

    _tprintf(TEXT("LPC-10 cycles per second: encode = %lu, decode = %lu\n"), lpc10e / count, lpc10d / count);
    _tprintf(TEXT("VBR-LPC-10 cycles per second: encode = %lu, decode = %lu\n"), vbrlpc10e / count, vbrlpc10d / count);
#endif
#if 1
    /* reset the input file for the OpenLPC 1.8 Kbps codec */
    lseek(infile, 0, SEEK_SET);
    type = readHeader(infile);
    len = OPENLPC_FRAMESIZE_1_8;
    count = 0;

    while(len == OPENLPC_FRAMESIZE_1_8)
    {
        int             encodedlen;
        short           samples[OPENLPC_FRAMESIZE_1_8];
        short           decoded[OPENLPC_FRAMESIZE_1_8];
        unsigned char   encoded[OPENLPC_FRAMESIZE_1_8];
        unsigned long   c;

        len = readSamples(infile, type, samples, OPENLPC_FRAMESIZE_1_8);
        count++;

        /* OpenLPC 1.8 Kbps */
        c = getcycles();
        encodedlen = openlpc_encode(samples, encoded, lpc18encode);
        lpc18e += countcycles(c);
        c = getcycles();
        (void)openlpc_decode(encoded, decoded, lpc18decode);
        lpc18d += countcycles(c);
        writeSamples(lpc18file, decoded, len);
    }
    count /= 8000/OPENLPC_FRAMESIZE_1_8;

    _tprintf(TEXT("OpenLPC 1.8 Kbps cycles per second: encode = %lu, decode = %lu\n"), lpc18e / count, lpc18d / count);

    /* reset the input file for the OpenLPC 1.4 Kbps codec */
    lseek(infile, 0, SEEK_SET);
    type = readHeader(infile);
    len = OPENLPC_FRAMESIZE_1_4;
    count = 0;

    while(len == OPENLPC_FRAMESIZE_1_4)
    {
        int             encodedlen;
        short           samples[OPENLPC_FRAMESIZE_1_4];
        short           decoded[OPENLPC_FRAMESIZE_1_4];
        unsigned char   encoded[OPENLPC_FRAMESIZE_1_4];
        unsigned long   c;

        len = readSamples(infile, type, samples, OPENLPC_FRAMESIZE_1_4);
        count++;

        /* OpenLPC 1.4 Kbps */
        c = getcycles();
        encodedlen = openlpc_encode(samples, encoded, lpc14encode);
        lpc14e += countcycles(c);
        c = getcycles();
        (void)openlpc_decode(encoded, decoded, lpc14decode);
        lpc14d += countcycles(c);
        writeSamples(lpc14file, decoded, len);
    }
    count /= 8000/OPENLPC_FRAMESIZE_1_4;

    _tprintf(TEXT("OpenLPC 1.4 Kbps cycles per second: encode = %lu, decode = %lu\n"), lpc14e / count, lpc14d / count);
#endif
#if 1
    /* reset the input file for the CELP 4.5 Kbps codec */
    celp45file = _topen(TEXT("celp45.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(celp45file);
    celpencode = create_celp_encoder_state();
    init_celp_encoder_state(celpencode, CELP_4_5_FRAMESIZE);
    celp_set_encoder_option(celpencode, CELP_CODEBOOK_LEN, CELP_CODEBOOK);
    celp_set_encoder_option(celpencode, CELP_FAST_GAIN, CELP_FAST);
    celpdecode = create_celp_decoder_state();
    init_celp_decoder_state(celpdecode, CELP_4_5_FRAMESIZE);

    lseek(infile, 0, SEEK_SET);
    type = readHeader(infile);
    len = CELP_4_5_FRAMESIZE;
    count = 0;

    while(len == CELP_4_5_FRAMESIZE)
    {
        int             encodedlen;
        short           samples[CELP_4_5_FRAMESIZE];
        short           decoded[CELP_4_5_FRAMESIZE];
        unsigned char   encoded[CELP_4_5_FRAMESIZE];
        unsigned long   c;

        len = readSamples(infile, type, samples, CELP_4_5_FRAMESIZE);
        count++;

        /* CELP */
        c = getcycles();
        encodedlen = celp_encode(samples, encoded, celpencode);
        celpe += countcycles(c);
        c = getcycles();
        (void)celp_decode(encoded, decoded, celpdecode);
        celpd += countcycles(c);
        writeSamples(celp45file, decoded, len);
    }
    count /= 8000/CELP_4_5_FRAMESIZE;

    _tprintf(TEXT("CELP 4.5 Kbps cycles per second: encode = %lu, decode = %lu\n"), celpe / count, celpd / count);
    close(celp45file);

    destroy_celp_encoder_state(celpencode);
    destroy_celp_decoder_state(celpdecode);

    /* reset the input file for the CELP 3.0 Kbps codec */
    celp30file = _topen(TEXT("celp30.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(celp30file);
    celpencode = create_celp_encoder_state();
    init_celp_encoder_state(celpencode, CELP_3_0_FRAMESIZE);
    celp_set_codebook_len(celpencode, CELP_CODEBOOK);
    celpdecode = create_celp_decoder_state();
    init_celp_decoder_state(celpdecode, CELP_3_0_FRAMESIZE);

    lseek(infile, 0, SEEK_SET);
    type = readHeader(infile);
    len = CELP_3_0_FRAMESIZE;
    celpe = celpd = count = 0;

    while(len == CELP_3_0_FRAMESIZE)
    {
        int             encodedlen;
        short           samples[CELP_3_0_FRAMESIZE];
        short           decoded[CELP_3_0_FRAMESIZE];
        unsigned char   encoded[CELP_3_0_FRAMESIZE];
        unsigned long   c;

        len = readSamples(infile, type, samples, CELP_3_0_FRAMESIZE);
        count++;

        /* CELP */
        c = getcycles();
        encodedlen = celp_encode(samples, encoded, celpencode);
        celpe += countcycles(c);
        c = getcycles();
        (void)celp_decode(encoded, decoded, celpdecode);
        celpd += countcycles(c);
        writeSamples(celp30file, decoded, len);
    }
    count /= 8000/CELP_3_0_FRAMESIZE;

    _tprintf(TEXT("CELP 3.0 Kbps cycles per second: encode = %lu, decode = %lu\n"), celpe / count, celpd / count);
    close(celp30file);

    destroy_celp_encoder_state(celpencode);
    destroy_celp_decoder_state(celpdecode);

    /* reset the input file for the CELP 2.3 Kbps codec */
    celp23file = _topen(TEXT("celp23.au"), O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    writeHeader(celp30file);
    celpencode = create_celp_encoder_state();
    init_celp_encoder_state(celpencode, CELP_2_3_FRAMESIZE);
    celp_set_codebook_len(celpencode, CELP_CODEBOOK);
    celpdecode = create_celp_decoder_state();
    init_celp_decoder_state(celpdecode, CELP_2_3_FRAMESIZE);

    lseek(infile, 0, SEEK_SET);
    type = readHeader(infile);
    len = CELP_2_3_FRAMESIZE;
    celpe = celpd = count = 0;

    while(len == CELP_2_3_FRAMESIZE)
    {
        int             encodedlen;
        short           samples[CELP_2_3_FRAMESIZE];
        short           decoded[CELP_2_3_FRAMESIZE];
        unsigned char   encoded[CELP_2_3_FRAMESIZE];
        unsigned long   c;

        len = readSamples(infile, type, samples, CELP_2_3_FRAMESIZE);
        count++;

        /* CELP */
        c = getcycles();
        encodedlen = celp_encode(samples, encoded, celpencode);
        celpe += countcycles(c);
        c = getcycles();
        (void)celp_decode(encoded, decoded, celpdecode);
        celpd += countcycles(c);
        writeSamples(celp23file, decoded, len);
    }
    count /= 8000/CELP_2_3_FRAMESIZE;

    _tprintf(TEXT("CELP 2.3 Kbps cycles per second: encode = %lu, decode = %lu\n"), celpe / count, celpd / count);
    close(celp23file);

    destroy_celp_encoder_state(celpencode);
    destroy_celp_decoder_state(celpdecode);
#endif
    destroy_openlpc_encoder_state(lpc14encode);
    destroy_openlpc_decoder_state(lpc14decode);

    destroy_openlpc_encoder_state(lpc18encode);
    destroy_openlpc_decoder_state(lpc18decode);

    destroy_lpc10_encoder_state(lpc10encode);
    destroy_lpc10_decoder_state(lpc10decode);

    destroy_lpc_encoder_state(lpcencode);
    destroy_lpc_decoder_state(lpcdecode);

    gsm_destroy(gsmencode);
    gsm_destroy(gsmdecode);

    close(lpc14file);
    close(lpc18file);
    close(lpc10file);
    close(lpcfile);
    close(gsmfile);
    close(adpcmfile);
    close(ulawfile);
    close(infile);
    return 0;
}

#if defined (_WIN32_WCE)
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPWSTR lpCmdLine, int nShowCmd )
{
	TCHAR	*argv[64], *tok; /* assume 64 max tokens */
	int		argc = 1, rval;
	TCHAR	namebuf[132];

	GetModuleFileName(NULL, namebuf, 132);

	argv[0] = namebuf;
	/* tokenize the command line */
	for (tok = _tcstok(lpCmdLine, TEXT(" \t")); tok; tok = _tcstok(NULL, TEXT(" \t")))
	{
		if (argc >= 64) break;
		argv[argc] = malloc(_tcslen(tok)+sizeof(TCHAR));
		_tcscpy(argv[argc], tok);
		argc++;
	}

	rval = _tmain(argc, argv);

	while(--argc)
	{
		free(argv[argc]);
	}
	return rval;
}
#endif

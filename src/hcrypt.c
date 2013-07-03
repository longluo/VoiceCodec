/*
  HawkEncrypt cross platform encryption library
  Copyright (C) 2003-2004 Phil Frisbie, Jr. (phil@hawksoft.com)

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

#ifdef WIN32

#ifdef _MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4214)
#endif /* _MSC_VER */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define _WIN32_WINNT 0x0400
#include <wincrypt.h>
#define getpid _getpid
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/timeb.h>
#endif

#ifndef MACOSX
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hcrypt.h"
#include "md5/md5.h"
#include "blowfish/blowfish.h"

typedef struct hcrypt_key_st
{
    BF_KEY          bf;
    unsigned char   iv[8];
    unsigned char   digest[16];
    int             n;
} hcrypt_key_t;

#if defined (_WIN32_WCE)
#ifdef _MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4214)
#endif /* _MSC_VER */

#include <winbase.h>

#ifdef _MSC_VER
#pragma warning (default:4201)
#pragma warning (default:4214)
#endif /* _MSC_VER */

struct timeb {
    time_t time;
    unsigned short millitm;
};

static void ftime( struct timeb *tb )
{
    SYSTEMTIME  st;
    int days, years, leapyears;

    if(tb == NULL)
    {
		nlSetError(NL_NULL_POINTER);
		return;
    }
    GetSystemTime(&st);
    leapyears = (st.wYear - 1970 + 1) / 4;
    years = st.wYear - 1970 - leapyears;

    days = years * 365 + leapyears * 366;

    switch (st.wMonth) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        days += 31;
        break;
    case 4:
    case 6:
    case 9:
    case 11:
        days += 30;
        break;
    case 2:
        days += (st.wYear%4 == 0) ? 29 : 28;
        break;
    default:
        break;
    }
    days += st.wDay;
    tb->time = days * 86400 + st.wHour * 3600 + st.wMinute * 60 + st.wSecond;
    tb->millitm = st.wMilliseconds;
}

#else /* !_WIN32_WCE*/

#ifdef HL_WINDOWS_APP
#include <sys/timeb.h>
#define ftime _ftime
#define timeb _timeb 
#else /* !HL_WINDOWS_APP */
#include <sys/time.h>
#endif /* HL_WINDOWS_APP */
#endif /* !_WIN32_WCE */


static unsigned long getcounter(void)
{
	unsigned long	cycles;

#if defined _MSC_VER && defined _M_IX86 

#define RDTSC __asm _emit 0x0f __asm _emit 0x31


__asm{
		RDTSC				; get the count
        xor eax, edx        ; XOR high 32 bits with low 32 bits
		mov cycles, eax		; copy register
	}

#elif defined __GNUC__ && defined __i386__

asm(
    " rdtsc ; "
    " xorl %%edx, %%eax ; "
    : "=a" (cycles)
    :
    : "edx", "memory"
    );

#else
    cycles = (unsigned long)rand() * (unsigned long)rand();

#endif

	return cycles;
}

HL_EXP /*&null&*/ hcrypt_salt* HL_APIENTRY hcryptNewSalt(void)
{
    static int          first = 1;
    struct MD5Context   md5c;
    /*&null&*/hcrypt_salt *salt = NULL;
    char                s[1024], *s2;
    struct timeb        t;
#ifdef WIN32
    HCRYPTPROV prov = 0;
#endif


    salt = (hcrypt_salt *)malloc(sizeof(hcrypt_salt));

    if(salt == NULL)
    {
        return NULL;
    }
    ftime(&t);
    if(first == 1)
    {
        srand(t.time / t.millitm);
        first = 0;
    }
    MD5Init(&md5c);

    /* use pseudo random information from server environment */
    /* to create a random 128 bit salt */
    sprintf(s, "%d", (int)t.time); /* will change every second as app is run */
    MD5Update(&md5c, (unsigned char *)s, (unsigned int)strlen(s));

    sprintf(s, "%u", t.millitm); /* will be fairly random depending on resolution */
    MD5Update(&md5c, (unsigned char *)s, (unsigned int)strlen(s));

    sprintf(s, "%lu", getcounter()); /* will be fairly random with Pentium class CPU */
    MD5Update(&md5c, (unsigned char *)s, (unsigned int)strlen(s));

    sprintf(s, "%d", (int)getpid()); /* may be different each time app is run */
    MD5Update(&md5c, (unsigned char *)s, (unsigned int)strlen(s));

    sprintf(s, "%d", (int)clock()); /* will slowly change as app is run */
    MD5Update(&md5c, (unsigned char *)s, (unsigned int)strlen(s));

    s2 = tmpnam(NULL); /* will be different with each call */
    if(s2 != NULL)
    {
        MD5Update(&md5c, (unsigned char *)s2, (unsigned int)strlen(s2));
    }

#ifdef WIN32
    /* just for good measure mix in some random chars from the Windows Crypt API */
    if(CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, 0))
    {
        BYTE    ran[256];

        if(CryptGenRandom(prov, sizeof(ran), ran))
        {
            MD5Update(&md5c, (unsigned char *)ran, (unsigned int)sizeof(ran));
        }
    }
    (void)CryptReleaseContext(prov, 0);
#endif

    MD5Final((unsigned char *)salt, &md5c);

    return salt;
}

HL_EXP void HL_APIENTRY hcryptDeleteKey(hcrypt_key *key)
{
    free(key);
}

HL_EXP /*&null&*/ hcrypt_key* HL_APIENTRY hcryptNewKey(const char *string, const hcrypt_salt* salt)
{
    struct MD5Context   md5c;
    /*&null&*/hcrypt_key  *bfkey = NULL;
    int                 loops = 10000;
    char                auth[] = "HawkVoiceDI authentication key";

    if(string == NULL)
    {
        return bfkey;
    }
    bfkey = malloc(sizeof(hcrypt_key));
    if(bfkey == NULL)
    {
        return bfkey;
    }
    memset(bfkey, 0, sizeof(hcrypt_key));
    MD5Init(&md5c);
    if(salt != NULL)
    {
        MD5Update(&md5c, (unsigned char *)salt, (unsigned int)sizeof(hcrypt_salt));
    }
    while(loops-- > 0)
    {
        MD5Update(&md5c, (unsigned char *)string, (unsigned int)strlen((char *)string));
    }
    /* finalize the encryption key */
    MD5Final(bfkey->digest, &md5c);
    BF_set_key(&bfkey->bf, 16, bfkey->digest);

    /* now create the internal authentication key, which is based on the encryption key */
    MD5Init(&md5c);
    MD5Update(&md5c, bfkey->digest, 16);
    MD5Update(&md5c, (unsigned char *)auth, (unsigned int)strlen(auth));
    /* finalize the internal authentication key */
    MD5Final(bfkey->digest, &md5c);
    return bfkey;
}

HL_EXP void HL_APIENTRY hcryptDeleteSalt(hcrypt_salt *key)
{
    free(key);
}

HL_EXP void HL_APIENTRY hcryptEncryptPacket(unsigned char *in, unsigned char *out, int buflen,
                                   hcrypt_key *key)
{
    unsigned char   iv[8];
    int             n = 0;

    memset(iv, 0, sizeof(iv));
    BF_cfb64_encrypt(in, out, buflen, &key->bf, iv, &n, BF_ENCRYPT);
}

HL_EXP void HL_APIENTRY hcryptEncryptStream(unsigned char *in, unsigned char *out, int buflen,
                                   hcrypt_key *key)
{
    BF_cfb64_encrypt(in, out, buflen, &key->bf, (unsigned char *)&key->iv, &key->n, BF_ENCRYPT);
}

HL_EXP void HL_APIENTRY hcryptDecryptPacket(unsigned char *in, unsigned char *out, int buflen,
                                   hcrypt_key *key)
{
    unsigned char   iv[8];
    int             n = 0;

    memset(iv, 0, sizeof(iv));
    BF_cfb64_encrypt(in, out, buflen, &key->bf, iv, &n, BF_DECRYPT);
}

HL_EXP void HL_APIENTRY hcryptDecryptStream(unsigned char *in, unsigned char *out, int buflen,
                                   hcrypt_key *key)
{
    BF_cfb64_encrypt(in, out, buflen, &key->bf, (unsigned char *)&key->iv, &key->n, BF_DECRYPT);
}

HL_EXP void HL_APIENTRY hcryptSignPacket(unsigned char *in, int buflen, hcrypt_key *key)
{
    struct MD5Context   md5c;
    unsigned char       sig[16];

    MD5Init(&md5c);
    MD5Update(&md5c, key->digest, 16);
    MD5Update(&md5c, in, buflen);
    MD5Final(sig, &md5c);
    in += buflen;
    /* fold down to 32 bits */
    in[0] = (unsigned char)(sig[0]^sig[15]^sig[8]^sig[7]);
    in[1] = (unsigned char)(sig[1]^sig[14]^sig[9]^sig[6]);
    in[2] = (unsigned char)(sig[2]^sig[13]^sig[10]^sig[5]);
    in[3] = (unsigned char)(sig[3]^sig[12]^sig[11]^sig[4]);
}

HL_EXP int HL_APIENTRY hcryptAuthenticatePacket(unsigned char *in, int buflen, hcrypt_key *key)
{
    struct MD5Context   md5c;
    unsigned char       sig[16];

    MD5Init(&md5c);
    MD5Update(&md5c, key->digest, 16);
    MD5Update(&md5c, in, buflen - 4);
    MD5Final(sig, &md5c);
    in += (buflen - 4);
    /* fold down to 32 bits */
    sig[0] = (unsigned char)(sig[0]^sig[15]^sig[8]^sig[7]);
    sig[1] = (unsigned char)(sig[1]^sig[14]^sig[9]^sig[6]);
    sig[2] = (unsigned char)(sig[2]^sig[13]^sig[10]^sig[5]);
    sig[3] = (unsigned char)(sig[3]^sig[12]^sig[11]^sig[4]);

    if( in[0] == sig[0] && in[1] == sig[1] &&
        in[2] == sig[2] && in[3] == sig[3] )
    {
        return 1;
    }
    return 0;
}


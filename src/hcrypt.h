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

#ifndef HCRYPT_H
#define HCRYPT_H

#include "hawklib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hcrypt_key_st hcrypt_key;

typedef struct hcrypt_salt_st {
    unsigned char   data[16]; /* 128 bit salt for encryption key generation */
} hcrypt_salt;


HL_EXP /*&null&*/ hcrypt_salt* HL_APIENTRY hcryptNewSalt(void);

HL_EXP void HL_APIENTRY hcryptDeleteKey(hcrypt_key *key);

HL_EXP /*&null&*/ hcrypt_key* HL_APIENTRY hcryptNewKey(const char *string, const hcrypt_salt* salt);

HL_EXP void HL_APIENTRY hcryptDeleteSalt(hcrypt_salt *key);

HL_EXP void HL_APIENTRY hcryptEncryptPacket(unsigned char *in, unsigned char *out, int buflen,
                                   hcrypt_key *key);

HL_EXP void HL_APIENTRY hcryptEncryptStream(unsigned char *in, unsigned char *out, int buflen,
                                   hcrypt_key *key);

HL_EXP void HL_APIENTRY hcryptDecryptPacket(unsigned char *in, unsigned char *out, int buflen,
                                   hcrypt_key *key);

HL_EXP void HL_APIENTRY hcryptDecryptStream(unsigned char *in, unsigned char *out, int buflen,
                                   hcrypt_key *key);

HL_EXP void HL_APIENTRY hcryptSignPacket(unsigned char *in, int buflen, hcrypt_key *key);

HL_EXP int HL_APIENTRY hcryptAuthenticatePacket(unsigned char *in, int buflen, hcrypt_key *key);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* HCRYPT_H */

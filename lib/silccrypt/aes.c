/* Modified for SILC -Pekka */
/* Includes key scheduling in C always, and encryption and decryption in C
   when assembler optimized version cannot be used. */
/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2006, Brian Gladman, Worcester, UK. All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products
      built using this software without specific written permission.

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue 09/09/2006
*/

#include "silccrypto.h"
#include "aes_internal.h"
#include "aes.h"

/*
 * SILC Crypto API for AES
 */

/* Sets the key for the cipher. */

SILC_CIPHER_API_SET_KEY(aes)
{
  switch (ops->mode) {
  case SILC_CIPHER_MODE_CTR:
  case SILC_CIPHER_MODE_CFB:
    aes_encrypt_key(key, keylen, &((AesContext *)context)->u.enc);
    break;

  case SILC_CIPHER_MODE_CBC:
  case SILC_CIPHER_MODE_ECB:
    if (encryption)
      aes_encrypt_key(key, keylen, &((AesContext *)context)->u.enc);
    else
      aes_decrypt_key(key, keylen, &((AesContext *)context)->u.dec);
    break;

  default:
    return FALSE;
  }
  return TRUE;
}

/* Sets IV for the cipher. */

SILC_CIPHER_API_SET_IV(aes)
{
  AesContext *aes = context;

  switch (ops->mode) {

  case SILC_CIPHER_MODE_CTR:
  case SILC_CIPHER_MODE_CFB:
    /* Starts new block. */
    aes->u.enc.inf.b[2] = 16;
    break;

  default:
    break;
  }
}

/* Initialize */

SILC_CIPHER_API_INIT(aes)
{
  AesContext *aes = silc_calloc(1, sizeof(AesContext));
  if (aes)
    aes->u.enc.inf.b[2] = 16;
}

/* Unnitialize */

SILC_CIPHER_API_UNINIT(aes)
{
  AesContext *aes = context;
  memset(aes, 0, sizeof(*aes));
  silc_free(aes);
}

/* Encrypts with the cipher. Source and destination buffers maybe one and
   same. */

SILC_CIPHER_API_ENCRYPT(aes)
{
  AesContext *aes = context;
  int i;

  switch (ops->mode) {
  case SILC_CIPHER_MODE_CTR:
    SILC_CTR_MSB_128_8(iv, cipher->block, aes->u.enc.inf.b[2], src, dst,
    		       aes_encrypt(iv, cipher->block, &aes->u.enc));
    break;

  case SILC_CIPHER_MODE_ECB:
    {
      SilcUInt32 nb = len >> 4;

      while (nb--) {
	aes_encrypt(src, dst, &aes->u.enc);
	src += 16;
	dst += 16;
      }
    }
    break;

  case SILC_CIPHER_MODE_CBC:
    {
      SilcUInt32 nb = len >> 4;

      SILC_ASSERT((len & (16 - 1)) == 0);
      if (len & (16 - 1))
	return FALSE;

      while(nb--) {
	lp32(iv)[0] ^= lp32(src)[0];
	lp32(iv)[1] ^= lp32(src)[1];
	lp32(iv)[2] ^= lp32(src)[2];
	lp32(iv)[3] ^= lp32(src)[3];
	aes_encrypt(iv, iv, &aes->u.enc);
	memcpy(dst, iv, 16);
	src += 16;
	dst += 16;
      }
    }
    break;

  case SILC_CIPHER_MODE_CFB:
    SILC_CFB_ENC_MSB_128_8(iv, aes->u.enc.inf.b[2], src, dst,
			   aes_encrypt(iv, iv, &aes->u.enc));
    break;

  default:
    return FALSE;
  }

  return TRUE;
}

/* Decrypts with the cipher. Source and destination buffers maybe one
   and same. */

SILC_CIPHER_API_DECRYPT(aes)
{
  AesContext *aes = context;

  switch (ops->mode) {
  case SILC_CIPHER_MODE_CTR:
    return silc_aes_encrypt(cipher, ops, context, src, dst, len, iv);
    break;

  case SILC_CIPHER_MODE_ECB:
    {
      SilcUInt32 nb = len >> 4;

      while (nb--) {
	aes_decrypt(src, dst, &aes->u.dec);
	src += 16;
	dst += 16;
      }
    }
    break;

  case SILC_CIPHER_MODE_CBC:
    {
      unsigned char tmp[16];
      SilcUInt32 nb = len >> 4;

      if (len & (16 - 1))
	return FALSE;

      while(nb--) {
	memcpy(tmp, src, 16);
	aes_decrypt(src, dst, &aes->u.dec);
	lp32(dst)[0] ^= lp32(iv)[0];
	lp32(dst)[1] ^= lp32(iv)[1];
	lp32(dst)[2] ^= lp32(iv)[2];
	lp32(dst)[3] ^= lp32(iv)[3];
	memcpy(iv, tmp, 16);
	src += 16;
	dst += 16;
      }
    }
    break;

  case SILC_CIPHER_MODE_CFB:
    SILC_CFB_DEC_MSB_128_8(iv, aes->u.enc.inf.b[2], src, dst,
			   aes_encrypt(iv, iv, &aes->u.enc));
    break;

  default:
    return FALSE;
  }

  return TRUE;
}

/****************************************************************************/

#if defined(__cplusplus)
extern "C"
{
#endif

#if defined( __WATCOMC__ ) && ( __WATCOMC__ >= 1100 )
#  define XP_DIR __cdecl
#else
#  define XP_DIR
#endif

#define d_1(t,n,b,e)       ALIGN const XP_DIR t n[256]    =   b(e)
#define d_4(t,n,b,e,f,g,h) ALIGN const XP_DIR t n[4][256] = { b(e), b(f), b(g), b(h) }
ALIGN const uint_32t t_dec(r,c)[RC_LENGTH] = rc_data(w0);

#ifdef SILC_AES_ASM
d_1(uint_8t, t_dec(i,box), isb_data, h0);
#endif /* SILC_AES_ASM */
d_4(uint_32t, t_dec(f,n), sb_data, u0, u1, u2, u3);
d_4(uint_32t, t_dec(f,l), sb_data, w0, w1, w2, w3);
d_4(uint_32t, t_dec(i,n), isb_data, v0, v1, v2, v3);
d_4(uint_32t, t_dec(i,l), isb_data, w0, w1, w2, w3);
d_4(uint_32t, t_dec(i,m), mm_data, v0, v1, v2, v3);

#define ke4(k,i) \
{   k[4*(i)+4] = ss[0] ^= ls_box(ss[3],3) ^ t_use(r,c)[i]; \
    k[4*(i)+5] = ss[1] ^= ss[0]; \
    k[4*(i)+6] = ss[2] ^= ss[1]; \
    k[4*(i)+7] = ss[3] ^= ss[2]; \
}

AES_RETURN aes_encrypt_key128(const unsigned char *key, aes_encrypt_ctx cx[1])
{   uint_32t    ss[4];

    cx->ks[0] = ss[0] = word_in(key, 0);
    cx->ks[1] = ss[1] = word_in(key, 1);
    cx->ks[2] = ss[2] = word_in(key, 2);
    cx->ks[3] = ss[3] = word_in(key, 3);

    ke4(cx->ks, 0);  ke4(cx->ks, 1);
    ke4(cx->ks, 2);  ke4(cx->ks, 3);
    ke4(cx->ks, 4);  ke4(cx->ks, 5);
    ke4(cx->ks, 6);  ke4(cx->ks, 7);
    ke4(cx->ks, 8);
    ke4(cx->ks, 9);
    cx->inf.b[0] = 10 * 16;
}

#define kef6(k,i) \
{   k[6*(i)+ 6] = ss[0] ^= ls_box(ss[5],3) ^ t_use(r,c)[i]; \
    k[6*(i)+ 7] = ss[1] ^= ss[0]; \
    k[6*(i)+ 8] = ss[2] ^= ss[1]; \
    k[6*(i)+ 9] = ss[3] ^= ss[2]; \
}

#define ke6(k,i) \
{   kef6(k,i); \
    k[6*(i)+10] = ss[4] ^= ss[3]; \
    k[6*(i)+11] = ss[5] ^= ss[4]; \
}

AES_RETURN aes_encrypt_key192(const unsigned char *key, aes_encrypt_ctx cx[1])
{   uint_32t    ss[6];

    cx->ks[0] = ss[0] = word_in(key, 0);
    cx->ks[1] = ss[1] = word_in(key, 1);
    cx->ks[2] = ss[2] = word_in(key, 2);
    cx->ks[3] = ss[3] = word_in(key, 3);
    cx->ks[4] = ss[4] = word_in(key, 4);
    cx->ks[5] = ss[5] = word_in(key, 5);

    ke6(cx->ks, 0);  ke6(cx->ks, 1);
    ke6(cx->ks, 2);  ke6(cx->ks, 3);
    ke6(cx->ks, 4);  ke6(cx->ks, 5);
    ke6(cx->ks, 6);
    kef6(cx->ks, 7);
    cx->inf.b[0] = 12 * 16;
}

#define kef8(k,i) \
{   k[8*(i)+ 8] = ss[0] ^= ls_box(ss[7],3) ^ t_use(r,c)[i]; \
    k[8*(i)+ 9] = ss[1] ^= ss[0]; \
    k[8*(i)+10] = ss[2] ^= ss[1]; \
    k[8*(i)+11] = ss[3] ^= ss[2]; \
}

#define ke8(k,i) \
{   kef8(k,i); \
    k[8*(i)+12] = ss[4] ^= ls_box(ss[3],0); \
    k[8*(i)+13] = ss[5] ^= ss[4]; \
    k[8*(i)+14] = ss[6] ^= ss[5]; \
    k[8*(i)+15] = ss[7] ^= ss[6]; \
}

AES_RETURN aes_encrypt_key256(const unsigned char *key, aes_encrypt_ctx cx[1])
{   uint_32t    ss[8];

    cx->ks[0] = ss[0] = word_in(key, 0);
    cx->ks[1] = ss[1] = word_in(key, 1);
    cx->ks[2] = ss[2] = word_in(key, 2);
    cx->ks[3] = ss[3] = word_in(key, 3);
    cx->ks[4] = ss[4] = word_in(key, 4);
    cx->ks[5] = ss[5] = word_in(key, 5);
    cx->ks[6] = ss[6] = word_in(key, 6);
    cx->ks[7] = ss[7] = word_in(key, 7);

    ke8(cx->ks, 0); ke8(cx->ks, 1);
    ke8(cx->ks, 2); ke8(cx->ks, 3);
    ke8(cx->ks, 4); ke8(cx->ks, 5);
    kef8(cx->ks, 6);
    cx->inf.b[0] = 14 * 16;
}

AES_RETURN aes_encrypt_key(const unsigned char *key, int key_len, aes_encrypt_ctx cx[1])
{
    switch(key_len)
    {
    case 16: case 128: aes_encrypt_key128(key, cx); return;
    case 24: case 192: aes_encrypt_key192(key, cx); return;
    case 32: case 256: aes_encrypt_key256(key, cx); return;
    }
}

#define v(n,i)  ((n) - (i) + 2 * ((i) & 3))
#define k4e(k,i) \
{   k[v(40,(4*(i))+4)] = ss[0] ^= ls_box(ss[3],3) ^ t_use(r,c)[i]; \
    k[v(40,(4*(i))+5)] = ss[1] ^= ss[0]; \
    k[v(40,(4*(i))+6)] = ss[2] ^= ss[1]; \
    k[v(40,(4*(i))+7)] = ss[3] ^= ss[2]; \
}

#define kdf4(k,i) \
{   ss[0] = ss[0] ^ ss[2] ^ ss[1] ^ ss[3]; \
    ss[1] = ss[1] ^ ss[3]; \
    ss[2] = ss[2] ^ ss[3]; \
    ss[4] = ls_box(ss[(i+3) % 4], 3) ^ t_use(r,c)[i]; \
    ss[i % 4] ^= ss[4]; \
    ss[4] ^= k[v(40,(4*(i)))];   k[v(40,(4*(i))+4)] = ff(ss[4]); \
    ss[4] ^= k[v(40,(4*(i))+1)]; k[v(40,(4*(i))+5)] = ff(ss[4]); \
    ss[4] ^= k[v(40,(4*(i))+2)]; k[v(40,(4*(i))+6)] = ff(ss[4]); \
    ss[4] ^= k[v(40,(4*(i))+3)]; k[v(40,(4*(i))+7)] = ff(ss[4]); \
}

#define kd4(k,i) \
{   ss[4] = ls_box(ss[(i+3) % 4], 3) ^ t_use(r,c)[i]; \
    ss[i % 4] ^= ss[4]; ss[4] = ff(ss[4]); \
    k[v(40,(4*(i))+4)] = ss[4] ^= k[v(40,(4*(i)))]; \
    k[v(40,(4*(i))+5)] = ss[4] ^= k[v(40,(4*(i))+1)]; \
    k[v(40,(4*(i))+6)] = ss[4] ^= k[v(40,(4*(i))+2)]; \
    k[v(40,(4*(i))+7)] = ss[4] ^= k[v(40,(4*(i))+3)]; \
}

#define kdl4(k,i) \
{   ss[4] = ls_box(ss[(i+3) % 4], 3) ^ t_use(r,c)[i]; ss[i % 4] ^= ss[4]; \
    k[v(40,(4*(i))+4)] = (ss[0] ^= ss[1]) ^ ss[2] ^ ss[3]; \
    k[v(40,(4*(i))+5)] = ss[1] ^ ss[3]; \
    k[v(40,(4*(i))+6)] = ss[0]; \
    k[v(40,(4*(i))+7)] = ss[1]; \
}

AES_RETURN aes_decrypt_key128(const unsigned char *key, aes_decrypt_ctx cx[1])
{   uint_32t    ss[5];
#if defined( d_vars )
        d_vars;
#endif
    cx->ks[v(40,(0))] = ss[0] = word_in(key, 0);
    cx->ks[v(40,(1))] = ss[1] = word_in(key, 1);
    cx->ks[v(40,(2))] = ss[2] = word_in(key, 2);
    cx->ks[v(40,(3))] = ss[3] = word_in(key, 3);

    kdf4(cx->ks, 0);  kd4(cx->ks, 1);
     kd4(cx->ks, 2);  kd4(cx->ks, 3);
     kd4(cx->ks, 4);  kd4(cx->ks, 5);
     kd4(cx->ks, 6);  kd4(cx->ks, 7);
     kd4(cx->ks, 8); kdl4(cx->ks, 9);
    cx->inf.b[0] = 10 * 16;
}

#define k6ef(k,i) \
{   k[v(48,(6*(i))+ 6)] = ss[0] ^= ls_box(ss[5],3) ^ t_use(r,c)[i]; \
    k[v(48,(6*(i))+ 7)] = ss[1] ^= ss[0]; \
    k[v(48,(6*(i))+ 8)] = ss[2] ^= ss[1]; \
    k[v(48,(6*(i))+ 9)] = ss[3] ^= ss[2]; \
}

#define k6e(k,i) \
{   k6ef(k,i); \
    k[v(48,(6*(i))+10)] = ss[4] ^= ss[3]; \
    k[v(48,(6*(i))+11)] = ss[5] ^= ss[4]; \
}

#define kdf6(k,i) \
{   ss[0] ^= ls_box(ss[5],3) ^ t_use(r,c)[i]; k[v(48,(6*(i))+ 6)] = ff(ss[0]); \
    ss[1] ^= ss[0]; k[v(48,(6*(i))+ 7)] = ff(ss[1]); \
    ss[2] ^= ss[1]; k[v(48,(6*(i))+ 8)] = ff(ss[2]); \
    ss[3] ^= ss[2]; k[v(48,(6*(i))+ 9)] = ff(ss[3]); \
    ss[4] ^= ss[3]; k[v(48,(6*(i))+10)] = ff(ss[4]); \
    ss[5] ^= ss[4]; k[v(48,(6*(i))+11)] = ff(ss[5]); \
}

#define kd6(k,i) \
{   ss[6] = ls_box(ss[5],3) ^ t_use(r,c)[i]; \
    ss[0] ^= ss[6]; ss[6] = ff(ss[6]); k[v(48,(6*(i))+ 6)] = ss[6] ^= k[v(48,(6*(i)))]; \
    ss[1] ^= ss[0]; k[v(48,(6*(i))+ 7)] = ss[6] ^= k[v(48,(6*(i))+ 1)]; \
    ss[2] ^= ss[1]; k[v(48,(6*(i))+ 8)] = ss[6] ^= k[v(48,(6*(i))+ 2)]; \
    ss[3] ^= ss[2]; k[v(48,(6*(i))+ 9)] = ss[6] ^= k[v(48,(6*(i))+ 3)]; \
    ss[4] ^= ss[3]; k[v(48,(6*(i))+10)] = ss[6] ^= k[v(48,(6*(i))+ 4)]; \
    ss[5] ^= ss[4]; k[v(48,(6*(i))+11)] = ss[6] ^= k[v(48,(6*(i))+ 5)]; \
}

#define kdl6(k,i) \
{   ss[0] ^= ls_box(ss[5],3) ^ t_use(r,c)[i]; k[v(48,(6*(i))+ 6)] = ss[0]; \
    ss[1] ^= ss[0]; k[v(48,(6*(i))+ 7)] = ss[1]; \
    ss[2] ^= ss[1]; k[v(48,(6*(i))+ 8)] = ss[2]; \
    ss[3] ^= ss[2]; k[v(48,(6*(i))+ 9)] = ss[3]; \
}

AES_RETURN aes_decrypt_key192(const unsigned char *key, aes_decrypt_ctx cx[1])
{   uint_32t    ss[7];
#if defined( d_vars )
        d_vars;
#endif
    cx->ks[v(48,(0))] = ss[0] = word_in(key, 0);
    cx->ks[v(48,(1))] = ss[1] = word_in(key, 1);
    cx->ks[v(48,(2))] = ss[2] = word_in(key, 2);
    cx->ks[v(48,(3))] = ss[3] = word_in(key, 3);

    cx->ks[v(48,(4))] = ff(ss[4] = word_in(key, 4));
    cx->ks[v(48,(5))] = ff(ss[5] = word_in(key, 5));
    kdf6(cx->ks, 0); kd6(cx->ks, 1);
    kd6(cx->ks, 2);  kd6(cx->ks, 3);
    kd6(cx->ks, 4);  kd6(cx->ks, 5);
    kd6(cx->ks, 6); kdl6(cx->ks, 7);
    cx->inf.b[0] = 12 * 16;
}

#define k8ef(k,i) \
{   k[v(56,(8*(i))+ 8)] = ss[0] ^= ls_box(ss[7],3) ^ t_use(r,c)[i]; \
    k[v(56,(8*(i))+ 9)] = ss[1] ^= ss[0]; \
    k[v(56,(8*(i))+10)] = ss[2] ^= ss[1]; \
    k[v(56,(8*(i))+11)] = ss[3] ^= ss[2]; \
}

#define k8e(k,i) \
{   k8ef(k,i); \
    k[v(56,(8*(i))+12)] = ss[4] ^= ls_box(ss[3],0); \
    k[v(56,(8*(i))+13)] = ss[5] ^= ss[4]; \
    k[v(56,(8*(i))+14)] = ss[6] ^= ss[5]; \
    k[v(56,(8*(i))+15)] = ss[7] ^= ss[6]; \
}

#define kdf8(k,i) \
{   ss[0] ^= ls_box(ss[7],3) ^ t_use(r,c)[i]; k[v(56,(8*(i))+ 8)] = ff(ss[0]); \
    ss[1] ^= ss[0]; k[v(56,(8*(i))+ 9)] = ff(ss[1]); \
    ss[2] ^= ss[1]; k[v(56,(8*(i))+10)] = ff(ss[2]); \
    ss[3] ^= ss[2]; k[v(56,(8*(i))+11)] = ff(ss[3]); \
    ss[4] ^= ls_box(ss[3],0); k[v(56,(8*(i))+12)] = ff(ss[4]); \
    ss[5] ^= ss[4]; k[v(56,(8*(i))+13)] = ff(ss[5]); \
    ss[6] ^= ss[5]; k[v(56,(8*(i))+14)] = ff(ss[6]); \
    ss[7] ^= ss[6]; k[v(56,(8*(i))+15)] = ff(ss[7]); \
}

#define kd8(k,i) \
{   ss[8] = ls_box(ss[7],3) ^ t_use(r,c)[i]; \
    ss[0] ^= ss[8]; ss[8] = ff(ss[8]); k[v(56,(8*(i))+ 8)] = ss[8] ^= k[v(56,(8*(i)))]; \
    ss[1] ^= ss[0]; k[v(56,(8*(i))+ 9)] = ss[8] ^= k[v(56,(8*(i))+ 1)]; \
    ss[2] ^= ss[1]; k[v(56,(8*(i))+10)] = ss[8] ^= k[v(56,(8*(i))+ 2)]; \
    ss[3] ^= ss[2]; k[v(56,(8*(i))+11)] = ss[8] ^= k[v(56,(8*(i))+ 3)]; \
    ss[8] = ls_box(ss[3],0); \
    ss[4] ^= ss[8]; ss[8] = ff(ss[8]); k[v(56,(8*(i))+12)] = ss[8] ^= k[v(56,(8*(i))+ 4)]; \
    ss[5] ^= ss[4]; k[v(56,(8*(i))+13)] = ss[8] ^= k[v(56,(8*(i))+ 5)]; \
    ss[6] ^= ss[5]; k[v(56,(8*(i))+14)] = ss[8] ^= k[v(56,(8*(i))+ 6)]; \
    ss[7] ^= ss[6]; k[v(56,(8*(i))+15)] = ss[8] ^= k[v(56,(8*(i))+ 7)]; \
}

#define kdl8(k,i) \
{   ss[0] ^= ls_box(ss[7],3) ^ t_use(r,c)[i]; k[v(56,(8*(i))+ 8)] = ss[0]; \
    ss[1] ^= ss[0]; k[v(56,(8*(i))+ 9)] = ss[1]; \
    ss[2] ^= ss[1]; k[v(56,(8*(i))+10)] = ss[2]; \
    ss[3] ^= ss[2]; k[v(56,(8*(i))+11)] = ss[3]; \
}

AES_RETURN aes_decrypt_key256(const unsigned char *key, aes_decrypt_ctx cx[1])
{   uint_32t    ss[9];
#if defined( d_vars )
        d_vars;
#endif
    cx->ks[v(56,(0))] = ss[0] = word_in(key, 0);
    cx->ks[v(56,(1))] = ss[1] = word_in(key, 1);
    cx->ks[v(56,(2))] = ss[2] = word_in(key, 2);
    cx->ks[v(56,(3))] = ss[3] = word_in(key, 3);

    cx->ks[v(56,(4))] = ff(ss[4] = word_in(key, 4));
    cx->ks[v(56,(5))] = ff(ss[5] = word_in(key, 5));
    cx->ks[v(56,(6))] = ff(ss[6] = word_in(key, 6));
    cx->ks[v(56,(7))] = ff(ss[7] = word_in(key, 7));
    kdf8(cx->ks, 0); kd8(cx->ks, 1);
    kd8(cx->ks, 2);  kd8(cx->ks, 3);
    kd8(cx->ks, 4);  kd8(cx->ks, 5);
    kdl8(cx->ks, 6);
    cx->inf.b[0] = 14 * 16;
}

AES_RETURN aes_decrypt_key(const unsigned char *key, int key_len, aes_decrypt_ctx cx[1])
{
    switch(key_len)
    {
    case 16: case 128: aes_decrypt_key128(key, cx); return;
    case 24: case 192: aes_decrypt_key192(key, cx); return;
    case 32: case 256: aes_decrypt_key256(key, cx); return;
    }
}

#ifndef SILC_AES_ASM
/* C version of AES */

#define si(y,x,k,c) (s(y,c) = word_in(x, c) ^ (k)[c])
#define so(y,x,c)   word_out(y, c, s(x,c))
#define locals(y,x)     x[4],y[4]
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
                        s(y,2) = s(x,2); s(y,3) = s(x,3);
#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); si(y,x,k,3)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3)

/* Visual C++ .Net v7.1 provides the fastest encryption code when using
   Pentium optimiation with small code but this is poor for decryption
   so we need to control this with the following VC++ pragmas
*/

#if defined( _MSC_VER ) && !defined( _WIN64 )
#pragma optimize( "s", on )
#endif

#define fwd_var(x,r,c)\
 ( r == 0 ? ( c == 0 ? s(x,0) : c == 1 ? s(x,1) : c == 2 ? s(x,2) : s(x,3))\
 : r == 1 ? ( c == 0 ? s(x,1) : c == 1 ? s(x,2) : c == 2 ? s(x,3) : s(x,0))\
 : r == 2 ? ( c == 0 ? s(x,2) : c == 1 ? s(x,3) : c == 2 ? s(x,0) : s(x,1))\
 :          ( c == 0 ? s(x,3) : c == 1 ? s(x,0) : c == 2 ? s(x,1) : s(x,2)))
#define fwd_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ four_tables(x,t_use(f,n),fwd_var,rf1,c))
#define fwd_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ four_tables(x,t_use(f,l),fwd_var,rf1,c))

AES_RETURN aes_encrypt(const unsigned char *in, unsigned char *out, const aes_encrypt_ctx cx[1])
{   uint_32t         locals(b0, b1);
    const uint_32t   *kp;

    kp = cx->ks;
    state_in(b0, in, kp);

    switch(cx->inf.b[0])
    {
    case 14 * 16:
        round(fwd_rnd,  b1, b0, kp + 1 * N_COLS);
        round(fwd_rnd,  b0, b1, kp + 2 * N_COLS);
        kp += 2 * N_COLS;
    case 12 * 16:
        round(fwd_rnd,  b1, b0, kp + 1 * N_COLS);
        round(fwd_rnd,  b0, b1, kp + 2 * N_COLS);
        kp += 2 * N_COLS;
    case 10 * 16:
        round(fwd_rnd,  b1, b0, kp + 1 * N_COLS);
        round(fwd_rnd,  b0, b1, kp + 2 * N_COLS);
        round(fwd_rnd,  b1, b0, kp + 3 * N_COLS);
        round(fwd_rnd,  b0, b1, kp + 4 * N_COLS);
        round(fwd_rnd,  b1, b0, kp + 5 * N_COLS);
        round(fwd_rnd,  b0, b1, kp + 6 * N_COLS);
        round(fwd_rnd,  b1, b0, kp + 7 * N_COLS);
        round(fwd_rnd,  b0, b1, kp + 8 * N_COLS);
        round(fwd_rnd,  b1, b0, kp + 9 * N_COLS);
        round(fwd_lrnd, b0, b1, kp +10 * N_COLS);
    }

    state_out(out, b0);
}

#define inv_var(x,r,c)\
 ( r == 0 ? ( c == 0 ? s(x,0) : c == 1 ? s(x,1) : c == 2 ? s(x,2) : s(x,3))\
 : r == 1 ? ( c == 0 ? s(x,3) : c == 1 ? s(x,0) : c == 2 ? s(x,1) : s(x,2))\
 : r == 2 ? ( c == 0 ? s(x,2) : c == 1 ? s(x,3) : c == 2 ? s(x,0) : s(x,1))\
 :          ( c == 0 ? s(x,1) : c == 1 ? s(x,2) : c == 2 ? s(x,3) : s(x,0)))

#define inv_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ four_tables(x,t_use(i,n),inv_var,rf1,c))
#define inv_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ four_tables(x,t_use(i,l),inv_var,rf1,c))
#define key_ofs     0
#define rnd_key(n)  (kp + n * N_COLS)

AES_RETURN aes_decrypt(const unsigned char *in, unsigned char *out, const aes_decrypt_ctx cx[1])
{   uint_32t        locals(b0, b1);
    const uint_32t *kp;

    kp = cx->ks + (key_ofs ? (cx->inf.b[0] >> 2) : 0);
    state_in(b0, in, kp);

    kp = cx->ks + (key_ofs ? 0 : (cx->inf.b[0] >> 2));
    switch(cx->inf.b[0])
    {
    case 14 * 16:
        round(inv_rnd,  b1, b0, rnd_key(-13));
        round(inv_rnd,  b0, b1, rnd_key(-12));
    case 12 * 16:
        round(inv_rnd,  b1, b0, rnd_key(-11));
        round(inv_rnd,  b0, b1, rnd_key(-10));
    case 10 * 16:
        round(inv_rnd,  b1, b0, rnd_key(-9));
        round(inv_rnd,  b0, b1, rnd_key(-8));
        round(inv_rnd,  b1, b0, rnd_key(-7));
        round(inv_rnd,  b0, b1, rnd_key(-6));
        round(inv_rnd,  b1, b0, rnd_key(-5));
        round(inv_rnd,  b0, b1, rnd_key(-4));
        round(inv_rnd,  b1, b0, rnd_key(-3));
        round(inv_rnd,  b0, b1, rnd_key(-2));
        round(inv_rnd,  b1, b0, rnd_key(-1));
        round(inv_lrnd, b0, b1, rnd_key( 0));
    }

    state_out(out, b0);
}

#if defined(__cplusplus)
}
#endif

#endif /* SILC_AES_ASM */

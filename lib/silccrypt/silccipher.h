/*

  silccipher.h

  Author: Pekka Riikonen <priikone@silcnet.org>

  Copyright (C) 1997 - 2008 Pekka Riikonen

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

*/

/****h* silccrypt/SILC Cipher Interface
 *
 * DESCRIPTION
 *
 * This is the interface for cipher functions.  It provides cipher
 * registering and unregistering routines, encryption and decryption
 * routines.
 *
 ***/

#ifndef SILCCIPHER_H
#define SILCCIPHER_H

/* Forward declarations */
typedef struct SilcCipherObjectStruct SilcCipherObject;

/****s* silccrypt/SilcCipherAPI/SilcCipher
 *
 * NAME
 *
 *    typedef struct SilcCipherStruct *SilcCipher;
 *
 * DESCRIPTION
 *
 *    This context is the actual cipher context and is allocated
 *    by silc_cipher_alloc and given as argument usually to all
 *    silc_cipher _* functions.  It is freed by the silc_cipher_free
 *    function.
 *
 ***/
typedef struct SilcCipherStruct *SilcCipher;

/****d* silccrypt/SilcCipherAPI/SilcCipherMode
 *
 * NAME
 *
 *    typedef enum { ... } SilcCipherMode;
 *
 * DESCRIPTION
 *
 *    Cipher modes.  Notes about cipher modes and implementation:
 *
 *    SILC_CIPHER_MODE_CBC
 *
 *      The Cipher-block Chaining mode.  The CBC is mode is a standard CBC
 *      mode.  The plaintext length must be multiple by the cipher block size.
 *      If it isn't the plaintext must be padded.
 *
 *    SILC_CIPHER_MODE_CTR
 *
 *      The Counter mode.  The CTR is normal counter mode.  The CTR mode does
 *      not require the plaintext length to be multiple by the cipher block
 *      size.  If the last plaintext block is shorter the remaining bits of
 *      the key stream are used next time silc_cipher_encrypt is called.  If
 *      silc_cipher_set_iv is called it will reset the counter for a new block
 *      (discarding any remaining bits from previous key stream).  The counter
 *      mode expects MSB first ordered counter.  Note also, the counter is
 *      incremented when silc_cipher_encrypt is called for the first time,
 *      before encrypting.
 *
 *    SILC_CIPHER_MODE_CFB
 *
 *      The Cipher Feedback mode.  The CFB mode is normal cipher feedback mode.
 *      The CFB mode does not require the plaintext length to be multiple by
 *      the cipher block size.  If the last plaintext block is shorter the
 *      remaining bits of the stream are used next time silc_cipher_encrypt is
 *      called.  If silc_cipher_set_iv is called it will reset the feedback
 *      for a new block (discarding any remaining bits from previous stream).
 *
 *    SILC_CIPHER_MODE_OFB
 *
 *      The Output Feedback mode.
 *
 *    SILC_CIPHER_MODE_ECB
 *
 *      The Electronic Codebook mode.  This mode does not provide sufficient
 *      security and should not be used.
 *
 *    Each mode modifies the IV (initialization vector) of the cipher when
 *    silc_cipher_encrypt or silc_cipher_decrypt is called.  The IV may be
 *    set/reset by calling silc_cipher_set_iv and the current IV can be
 *    retrieved by calling silc_cipher_get_iv.
 *
 * SOURCE
 */
typedef enum {
  SILC_CIPHER_MODE_ECB = 1,	/* ECB mode */
  SILC_CIPHER_MODE_CBC = 2,	/* CBC mode */
  SILC_CIPHER_MODE_CTR = 3,	/* CTR mode */
  SILC_CIPHER_MODE_CFB = 4,	/* CFB mode */
  SILC_CIPHER_MODE_OFB = 5,	/* OFB mode */
} SilcCipherMode;
/***/

#define SILC_CIPHER_MAX_IV_SIZE 16		/* Maximum IV size */
#define SILC_DEFAULT_CIPHER "aes-256-cbc"	/* Default cipher */

/* Marks for all ciphers in silc. This can be used in silc_cipher_unregister
   to unregister all ciphers at once. */
#define SILC_ALL_CIPHERS ((SilcCipherObject *)1)

#include "silccipher_i.h"

/* Static list of ciphers for silc_cipher_register_default(). */
extern DLLAPI const SilcCipherObject silc_default_ciphers[];

/* Prototypes */

/****f* silccrypt/SilcCipherAPI/silc_cipher_register
 *
 * SYNOPSIS
 *
 *    SilcBool silc_cipher_register(const SilcCipherObject *cipher);
 *
 * DESCRIPTION
 *
 *    Register a new cipher into SILC. This is used at the initialization of
 *    the SILC. This function allocates a new object for the cipher to be
 *    registered. Therefore, if memory has been allocated for the object sent
 *    as argument it has to be free'd after this function returns succesfully.
 *
 ***/
SilcBool silc_cipher_register(const SilcCipherObject *cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_unregister
 *
 * SYNOPSIS
 *
 *    SilcBool silc_cipher_unregister(SilcCipherObject *cipher);
 *
 * DESCRIPTION
 *
 *    Unregister a cipher from the SILC.
 *
 ***/
SilcBool silc_cipher_unregister(SilcCipherObject *cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_register_default
 *
 * SYNOPSIS
 *
 *    SilcBool silc_cipher_register_default(void);
 *
 * DESCRIPTION
 *
 *    Function that registers all the default ciphers (all builtin ciphers).
 *    The application may use this to register the default ciphers if specific
 *    ciphers in any specific order is not wanted.
 *
 ***/
SilcBool silc_cipher_register_default(void);

/****f* silccrypt/SilcCipherAPI/silc_cipher_unregister_all
 *
 * SYNOPSIS
 *
 *    SilcBool silc_cipher_unregister_all(void);
 *
 * DESCRIPTION
 *
 *    Unregisters all ciphers.
 *
 ***/
SilcBool silc_cipher_unregister_all(void);

/****f* silccrypt/SilcCipherAPI/silc_cipher_alloc
 *
 * SYNOPSIS
 *
 *    SilcBool silc_cipher_alloc(const char *name,
 *                               SilcCipher *new_cipher);
 *
 * DESCRIPTION
 *
 *    Allocates a new SILC cipher object. Function returns TRUE on succes 
 *    and FALSE on error. The allocated cipher is returned in new_cipher
 *    argument. The caller must set the key to the cipher after this
 *    function has returned by calling the silc_cipher_set_key.
 *
 *    The following ciphers are supported:
 *
 *    aes-256-ctr            AES-256, Counter mode
 *    aes-192-ctr            AES-192, Counter mode
 *    aes-128-ctr            AES,128, Counter mode
 *    aes-256-cbc            AES-256, Cipher block chaining mode
 *    aes-192-cbc            AES-192, Cipher block chaining mode
 *    aes-128-cbc            AES,128, Cipher block chaining mode
 *    twofish-256-cbc        Twofish-256, Cipher block chaining mode
 *    twofish-192-cbc        Twofish-192, Cipher block chaining mode
 *    twofish-128-cbc        Twofish-128, Cipher block chaining mode
 *
 *    Notes about modes:
 *
 ***/
SilcBool silc_cipher_alloc(const char *name, SilcCipher *new_cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_alloc
 *
 * SYNOPSIS
 *
 *    SilcBool silc_cipher_alloc_full(const char *alg_name,
 *                                    SilcUInt32 key_len,
 *                                    SilcCipherMode mode,
 *                                    SilcCipher *new_cipher);
 * DESCRIPTION
 *
 *    Same as silc_cipher_alloc but takes the cipher algorithm name,
 *    key length and mode as separate arguments.  
 *
 ***/
SilcBool silc_cipher_alloc_full(const char *alg_name, SilcUInt32 key_len,
				SilcCipherMode mode, SilcCipher *new_cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_free
 *
 * SYNOPSIS
 *
 *    void silc_cipher_free(SilcCipher cipher);
 *
 * DESCRIPTION
 *
 *    Frees the given cipher.
 *
 ***/
void silc_cipher_free(SilcCipher cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_is_supported
 *
 * SYNOPSIS
 *
 * SilcBool silc_cipher_is_supported(const char *name);
 *
 * DESCRIPTION
 *
 *    Returns TRUE if cipher `name' is supported.
 *
 ***/
SilcBool silc_cipher_is_supported(const char *name);

/****f* silccrypt/SilcCipherAPI/silc_cipher_get_supported
 *
 * SYNOPSIS
 *
 *    char *silc_cipher_get_supported(SilcBool only_registered);
 *
 * DESCRIPTION
 *
 *    Returns comma separated list of supported ciphers.  If `only_registered'
 *    is TRUE only ciphers explicitly registered with silc_cipher_register
 *    are returned.  If FALSE, then all registered and default builtin
 *    ciphers are returned.  However, if there are no registered ciphers
 *    and `only_registered' is TRUE, the builtin ciphers are returned.
 *
 ***/
char *silc_cipher_get_supported(SilcBool only_registered);

/****f* silccrypt/SilcCipherAPI/silc_cipher_encrypt
 *
 * SYNOPSIS
 *
 *    SilcBool silc_cipher_encrypt(SilcCipher cipher,
 *                                 const unsigned char *src,
 *                                 unsigned char *dst, SilcUInt32 len,
 *                                 unsigned char *iv);
 *
 * DESCRIPTION
 *
 *    Encrypts data from `src' into `dst' with the specified cipher and
 *    Initial Vector (IV).  If the `iv' is NULL then the cipher's internal
 *    IV is used.  The `src' and `dst' maybe same buffer.
 *
 ***/
SilcBool silc_cipher_encrypt(SilcCipher cipher, const unsigned char *src,
			     unsigned char *dst, SilcUInt32 len,
			     unsigned char *iv);

/****f* silccrypt/SilcCipherAPI/silc_cipher_decrypt
 *
 * SYNOPSIS
 *
 *    SilcBool silc_cipher_decrypt(SilcCipher cipher,
 *                                 const unsigned char *src,
 *                                 unsigned char *dst, SilcUInt32 len,
 *                                 unsigned char *iv);
 *
 * DESCRIPTION
 *
 *    Decrypts data from `src' into `dst' with the specified cipher and
 *    Initial Vector (IV).  If the `iv' is NULL then the cipher's internal
 *    IV is used.  The `src' and `dst' maybe same buffer.
 *
 ***/
SilcBool silc_cipher_decrypt(SilcCipher cipher, const unsigned char *src,
			     unsigned char *dst, SilcUInt32 len,
			     unsigned char *iv);

/****f* silccrypt/SilcCipherAPI/silc_cipher_set_key
 *
 * SYNOPSIS
 *
 *    SilcBool silc_cipher_set_key(SilcCipher cipher, const unsigned char *key,
 *                                 SilcUInt32 bit_keylen, SilcBool encryption);
 *
 * DESCRIPTION
 *
 *    Sets the key for the cipher.  The `keylen' is the key length in
 *    bits.  If the `encryption' is TRUE the key is for encryption, if FALSE
 *    the key is for decryption.
 *
 ***/
SilcBool silc_cipher_set_key(SilcCipher cipher, const unsigned char *key,
			     SilcUInt32 bit_keylen, SilcBool encryption);

/****f* silccrypt/SilcCipherAPI/silc_cipher_set_iv
 *
 * SYNOPSIS
 *
 *    void silc_cipher_set_iv(SilcCipher cipher, const unsigned char *iv);
 *
 * DESCRIPTION
 *
 *    Sets the IV (initialization vector) for the cipher.  The `iv' must be
 *    the size of the block size of the cipher.  If `iv' is NULL this
 *    does not do anything.
 *
 *    If the encryption mode is CTR (Counter mode) this also resets the
 *    the counter for a new block.  This is done also if `iv' is NULL.
 *
 *    If the encryption mode is CFB (cipher feedback) this also resets the
 *    the feedback stream for a new block.  This is done also if `iv' is NULL.
 *
 ***/
void silc_cipher_set_iv(SilcCipher cipher, const unsigned char *iv);

/****f* silccrypt/SilcCipherAPI/silc_cipher_get_iv
 *
 * SYNOPSIS
 *
 *    unsigned char *silc_cipher_get_iv(SilcCipher cipher);
 *
 * DESCRIPTION
 *
 *    Returns the IV (initial vector) of the cipher.  The returned
 *    pointer must not be freed by the caller.  If the caller modifies
 *    the returned pointer the IV inside cipher is also modified.
 *
 ***/
unsigned char *silc_cipher_get_iv(SilcCipher cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_get_key_len
 *
 * SYNOPSIS
 *
 *    SilcUInt32 silc_cipher_get_key_len(SilcCipher cipher);
 *
 * DESCRIPTION
 *
 *    Returns the key length of the cipher in bits.
 *
 ***/
SilcUInt32 silc_cipher_get_key_len(SilcCipher cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_get_block_len
 *
 * SYNOPSIS
 *
 *    SilcUInt32 silc_cipher_get_block_len(SilcCipher cipher);
 *
 * DESCRIPTION
 *
 *    Returns the block size of the cipher in bytes.
 *
 ***/
SilcUInt32 silc_cipher_get_block_len(SilcCipher cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_get_iv_len
 *
 * SYNOPSIS
 *
 *    SilcUInt32 silc_cipher_get_iv_len(SilcCipher cipher);
 *
 * DESCRIPTION
 *
 *    Returns the IV length of the cipher in bytes.
 *
 ***/
SilcUInt32 silc_cipher_get_iv_len(SilcCipher cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_get_name
 *
 * SYNOPSIS
 *
 *    const char *silc_cipher_get_name(SilcCipher cipher);
 *
 * DESCRIPTION
 *
 *    Returns the full name of the cipher (eg. 'aes-256-ctr').
 *
 ***/
const char *silc_cipher_get_name(SilcCipher cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_get_alg_name
 *
 * SYNOPSIS
 *
 *    const char *silc_cipher_get_alg_name(SilcCipher cipher);
 *
 * DESCRIPTION
 *
 *    Returns the algorithm name of the cipher (eg. 'aes').
 *
 ***/
const char *silc_cipher_get_alg_name(SilcCipher cipher);

/****f* silccrypt/SilcCipherAPI/silc_cipher_get_mode
 *
 * SYNOPSIS
 *
 *    SilcCipherMode silc_cipher_get_mode(SilcCipher cipher);
 *
 * DESCRIPTION
 *
 *    Returns the cipher mode.
 *
 ***/
SilcCipherMode silc_cipher_get_mode(SilcCipher cipher);

#endif /* SILCCIPHER_H */

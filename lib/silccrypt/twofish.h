/*

  twofish.h

  Author: Pekka Riikonen <priikone@silcnet.org>

  Copyright (C) 2008 Pekka Riikonen

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

*/

#ifndef TWOFISH_H
#define TWOFISH_H

/*
 * SILC Crypto API for Twofish
 */

SILC_CIPHER_API_SET_KEY(twofish);
SILC_CIPHER_API_SET_IV(twofish);
SILC_CIPHER_API_INIT(twofish);
SILC_CIPHER_API_UNINIT(twofish);
SILC_CIPHER_API_ENCRYPT(twofish);
SILC_CIPHER_API_DECRYPT(twofish);

#endif /* TWOFISH_H */

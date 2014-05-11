/*
 * AES key unwrap (128-bit KEK, RFC3394)
 *
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "aes.h"

/**
 * aes_unwrap - Unwrap key with AES Key Wrap Algorithm (128-bit KEK) (RFC3394)
 * @kek: Key encryption key (KEK)
 * @n: Length of the plaintext key in 64-bit units; e.g., 2 = 128-bit = 16
 * bytes
 * @cipher: Wrapped key to be unwrapped, (n + 1) * 64 bits
 * @plain: Plaintext key, n * 64 bits
 * Returns: 0 on success, -1 on failure (e.g., integrity verification failed)
 */
int aes_unwrap(const aes_uchar *kek, int n, const aes_uchar *cipher, aes_uchar *plain)
{
	aes_uchar a[8], *r, b[16];
	int i, j;
	void *ctx;

	/* 1) Initialize variables. */
	memcpy(a, cipher, 8);
	r = plain;
	memcpy(r, cipher + 8, 8 * n);

	ctx = aes_decrypt_init(kek, 16);
	if (ctx == NULL)
		return -1;

	/* 2) Compute intermediate values.
	 * For j = 5 to 0
	 *     For i = n to 1
	 *         B = AES-1(K, (A ^ t) | R[i]) where t = n*j+i
	 *         A = MSB(64, B)
	 *         R[i] = LSB(64, B)
	 */
	for (j = 5; j >= 0; j--) {
		r = plain + (n - 1) * 8;
		for (i = n; i >= 1; i--) {
			memcpy(b, a, 8);
			b[7] ^= n * j + i;

			memcpy(b + 8, r, 8);
			aes_decrypt(ctx, b, b);
			memcpy(a, b, 8);
			memcpy(r, b + 8, 8);
			r -= 8;
		}
	}
	aes_decrypt_deinit(ctx);

	/* 3) Output results.
	 *
	 * These are already in @plain due to the location of temporary
	 * variables. Just verify that the IV matches with the expected value.
	 */
	for (i = 0; i < 8; i++) {
		if (a[i] != 0xa6)
			return -1;
	}

	return 0;
}

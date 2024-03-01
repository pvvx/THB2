/*
 * ccm.c
 */

#include <stdint.h>
#include <config.h>
#if (DEV_SERVICES & SERVICE_BINDKEY)
#include "ccm.h"

extern void LL_ENC_AES128_Encrypt1( unsigned char * key,
		unsigned char * plaintext,
		unsigned char * ciphertext );
/*
 * Macros for common operations.
 * Results in smaller compiled code than static inline functions.
 */

/*
 * Update the CBC-MAC state in y using a block in b
 * (Always using b as the source helps the compiler optimise a bit better.)
 */
#define UPDATE_CBC_MAC          \
    for (i = 0; i < 16; i++)  	\
        y[i] ^= b[i];           \
    LL_ENC_AES128_Encrypt1((unsigned char *)key, y, y);

/*
 * Encrypt or decrypt a partial block with CTR
 * Warning: using b for temporary storage! src and dst must not be b!
 * This avoids allocating one more 16 bytes buffer while allowing src == dst.
 */
#define CTR_CRYPT(dst, src, len)	\
	LL_ENC_AES128_Encrypt1((unsigned char *)key, ctr, b);	\
    for (i = 0; i < len; i++)		\
        dst[i] = src[i] ^ b[i];

/*
 * Authenticated encryption or decryption
 */
int ccm_auth_crypt( int mode, const unsigned char *key,
                           const unsigned char *iv, size_t iv_len,
                           const unsigned char *input, size_t length,
                           unsigned char *output,
                           unsigned char *tag, size_t tag_len )
{
    unsigned char i;
    unsigned char q;
    size_t len_left;
    unsigned char b[16];
    unsigned char y[16];
    unsigned char ctr[16];
    const unsigned char *src;
    unsigned char *dst;

    q = 16 - 1 - (unsigned char) iv_len;
	b[0] = ((tag_len - 2) / 2) << 3;
	b[0] |= q - 1;
	memcpy(b + 1, iv, iv_len);
	for (i = 0, len_left = length; i < q; i++, len_left >>= 8)
		b[15 - i] = (unsigned char) (len_left & 0xFF);
	if (len_left > 0)
		return (-1);
	memset(y, 0, 16);
	UPDATE_CBC_MAC;
    ctr[0] = q - 1;
    memcpy( ctr + 1, iv, iv_len );
    memset( ctr + 1 + iv_len, 0, q );
    ctr[15] = 1;
    len_left = length;
    src = input;
    dst = output;
    while (len_left > 0) {
		size_t use_len = len_left > 16 ? 16 : len_left;
		if (mode == CCM_ENCRYPT) {
			memset(b, 0, 16);
			memcpy(b, src, use_len);
			UPDATE_CBC_MAC;
		}
		CTR_CRYPT( dst, src, use_len );
		if (mode == CCM_DECRYPT) {
			memset(b, 0, 16);
			memcpy(b, dst, use_len);
			UPDATE_CBC_MAC;
		}
		dst += use_len;
		src += use_len;
		len_left -= use_len;
        for (i = 0; i < q; i++)
			if (++ctr[15 - i] != 0)
				break;
    }
    for (i = 0; i < q; i++)
		ctr[15 - i] = 0;
	CTR_CRYPT( y, y, 16 );
	memcpy(tag, y, tag_len);
	return (0);
}

/*
 * Authenticated decryption
 */
int aes_ccm_decrypt( const unsigned char *key,
                      const unsigned char *iv, size_t iv_len,
                      const unsigned char *input, size_t length,
                      unsigned char *output,
                      const unsigned char *tag, size_t tag_len )
{
    int ret;
    unsigned char check_tag[16];
    unsigned char i;
    int diff;
    if( ( ret = ccm_auth_crypt( CCM_DECRYPT, key,
                                iv, iv_len,
								input, length,
								output,
								check_tag, tag_len ) ) != 0 )
    {
        return(ret);
    }

    for( diff = 0, i = 0; i < tag_len; i++ )
        diff |= tag[i] ^ check_tag[i];

    if( diff != 0 )
    {
        volatile unsigned char *p = output; while(length-- ) *p++ = 0;
        return(-1);
    }

    return(0);
}

#endif // (DEV_SERVICES & SERVICE_BINDKEY)

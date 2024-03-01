/**
 * \file ccm.h
 *
 */
#ifndef _CCM_H_
#define _CCM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CCM_ENCRYPT 0
#define CCM_DECRYPT 1
/*
 * Authenticated encryption or decryption
 */
int ccm_auth_crypt( int mode, const unsigned char *key,
                           const unsigned char *iv, size_t iv_len,
                           const unsigned char *input, size_t length,
                           unsigned char *output,
                           unsigned char *tag, size_t tag_len );
/**
 * \brief           CCM buffer authenticated decryption
 *
 * \param key       key must be 16 bytes
 * \param length    length of the input data
 * \param iv        initialization vector
 * \param iv_len    length of IV
 * \param input     buffer holding the input data
 * \param output    buffer for holding the output data
 * \param tag       buffer holding the tag
 * \param tag_len   length of the tag
 *
 * \return         0 if successful and authenticated,
 *                 MBEDTLS_ERR_CCM_AUTH_FAILED if tag does not match
 */
int aes_ccm_decrypt( const unsigned char *key,
                      const unsigned char *iv, size_t iv_len,
                      const unsigned char *input, size_t length,
                      unsigned char *output,
                      const unsigned char *tag, size_t tag_len );

#ifdef __cplusplus
}
#endif

#endif /* _CCM_H_ */

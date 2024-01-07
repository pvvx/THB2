To realize the AES-CCM functionality using the aes.c interface herewith, pull the files aes-ccm.c and aes.h from the repo: https://github.com/wi-fi-analyzer/crackle, and make the following updates -

** Update the 'xor_aes_block()' function implementation as below and use.

static void xor_aes_block(u8 *dst, const u8 *src)
{
#ifdef HAVE_ALIGNED_MEM_OPERATION
    u32 *d = (u32 *) dst;
    u32 *s = (u32 *) src;
    *d++ ^= *s++;
    *d++ ^= *s++;
    *d++ ^= *s++;
    *d++ ^= *s++;
#else /* HAVE_ALIGNED_MEM_OPERATION */
    u32 i;

    for (i = 0; i < 16; i++)
    {
        *dst++ ^= *src++;
    }
#endif /* HAVE_ALIGNED_MEM_OPERATION */
}


** Update the 'aes_ccm_encr()' function implementation as below and use.

static void aes_ccm_encr(void *aes, size_t L, const u8 *in, size_t len, u8 *out,
                         u8 *a)
{
    size_t last = len % AES_BLOCK_SIZE;
    size_t i;

    /* crypt = msg XOR (S_1 | S_2 | ... | S_n) */
    for (i = 1; i <= len / AES_BLOCK_SIZE; i++) {
        PUT_BE16(&a[AES_BLOCK_SIZE - 2], i);
        /* S_i = E(K, A_i) */
        aes_encrypt(aes, a, out);
        xor_aes_block(out, in);
        out += AES_BLOCK_SIZE;
        in += AES_BLOCK_SIZE;
    }
#if 0
    if (last) {
        PUT_BE16(&a[AES_BLOCK_SIZE - 2], i);
        aes_encrypt(aes, a, out);
        /* XOR zero-padded last block */
        for (i = 0; i < last; i++)
            *out++ ^= *in++;
    }
#else /* 0 */
    if (last) {
        u8 tout[AES_BLOCK_SIZE];

        PUT_BE16(&a[AES_BLOCK_SIZE - 2], i);
        aes_encrypt(aes, a, tout);
        /* XOR zero-padded last block */
        for (i = 0; i < last; i++)
            *out++ = tout[i] ^ *in++;
    }
#endif /* 0 */
}

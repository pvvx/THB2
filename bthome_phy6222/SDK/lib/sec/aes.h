/*************************************************************
 aes.h

 SDK_LICENSE

**************************************************************/

#ifndef AES_H
#define AES_H

#include "bcomdef.h"

#define AES_DEBUG           0

typedef enum
{
    AES_ENC,
    AES_DEC,
} AES_MODE_e;




bool            finidv(void);
void            getBit(unsigned char ci,int* b);
unsigned char   getByte(int* b);
unsigned char   xtime(unsigned char a,int xtm);
void            printfStats(unsigned char state[][4],int mode);
int             aes(const unsigned char* key,unsigned char* key_o,unsigned char* dat_i,unsigned char* dat_o, int mode);
void            KeyExpansion(const unsigned char* key, unsigned char w[][4][4],int mode);
unsigned char   FFmul(unsigned char a, unsigned char b);

void            SubBytes(unsigned char state[][4],int mode);
void            ShiftRows(unsigned char state[][4],int mode);
void            MixColumns(unsigned char state[][4], int mode);
void            AddRoundKey(unsigned char state[][4], unsigned char k[][4]);

void            xor128bit(unsigned char* x,unsigned char* y,unsigned char* z);
//bool            aes_ccm_phyplus_dec(const unsigned char* iv, unsigned char* din,int dLen, unsigned char* micIn,
//unsigned char*dout);
#endif




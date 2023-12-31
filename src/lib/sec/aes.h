/**************************************************************************************************

    Phyplus Microelectronics Limited confidential and proprietary.
    All rights reserved.

    IMPORTANT: All rights of this software belong to Phyplus Microelectronics
    Limited ("Phyplus"). Your use of this Software is limited to those
    specific rights granted under  the terms of the business contract, the
    confidential agreement, the non-disclosure agreement and any other forms
    of agreements as a customer or a partner of Phyplus. You may not use this
    Software unless you agree to abide by the terms of these agreements.
    You acknowledge that the Software may not be modified, copied,
    distributed or disclosed unless embedded on a Phyplus Bluetooth Low Energy
    (BLE) integrated circuit, either as a product or is integrated into your
    products.  Other than for the aforementioned purposes, you may not use,
    reproduce, copy, prepare derivative works of, modify, distribute, perform,
    display or sell this Software and/or its documentation for any purposes.

    YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
    PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
    INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
    NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
    PHYPLUS OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
    NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
    LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
    OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
    OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

**************************************************************************************************/

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




/*****************************************************************************
 aes.c

 SDK_LICENSE

*****************************************************************************/

#include "aes.h"
#include "flash.h"
#include "ll_def.h"
#include "ll_enc.h"
//extern chipId_t g_chipId;
extern const char* s_company_id;

unsigned char sBox[256] =
{
    /*  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f */
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76, /*0*/
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0, /*1*/
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15, /*2*/
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75, /*3*/
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84, /*4*/
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf, /*5*/
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8, /*6*/
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2, /*7*/
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73, /*8*/
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb, /*9*/
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79, /*a*/
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08, /*b*/
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a, /*c*/
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e, /*d*/
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf, /*e*/
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16  /*f*/
};
unsigned char invsBox[256] =
{
    /*  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f  */
    0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb, /*0*/
    0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb, /*1*/
    0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e, /*2*/
    0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25, /*3*/
    0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92, /*4*/
    0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84, /*5*/
    0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06, /*6*/
    0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b, /*7*/
    0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73, /*8*/
    0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e, /*9*/
    0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b, /*a*/
    0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4, /*b*/
    0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f, /*c*/
    0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef, /*d*/
    0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61, /*e*/
    0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d  /*f*/
};


void getBit(unsigned char ci,int* b)
{
    for(int i=0; i<8; i++)
        b[i]=(ci>>i)&0x01;
}


unsigned char getByte(int* b)
{
    unsigned char out=0;

    for(int i=0; i<8; i++)
        out=out+(b[i]<<i);

    return out;
}

unsigned char xtime(unsigned char a,int xtm)
{
    int b[8]= {0};
    int ao[8]= {0};
    getBit(a,b);

    if(xtm==1)
    {
        ao[0]=0   ^b[7];
        ao[1]=b[0]^b[7];
        ao[2]=b[1];
        ao[3]=b[2]^b[7];
        ao[4]=b[3]^b[7];
        ao[5]=b[4];
        ao[6]=b[5];
        ao[7]=b[6];
    }
    else if(xtm==2)
    {
        ao[0]=b[6];
        ao[1]=b[6]^b[7];
        ao[2]=b[0]^b[7];
        ao[3]=b[1]^b[6];
        ao[4]=b[2]^(b[6]^b[7]);
        ao[5]=b[3]^b[7];
        ao[6]=b[4];
        ao[7]=b[5];
    }
    else if(xtm==3)
    {
        ao[0]=b[5];
        ao[1]=b[5]^b[6];
        ao[2]=b[6]^b[7];
        ao[3]=b[0]^(b[5]^b[7]);
        ao[4]=b[1]^(b[5]^b[6]);
        ao[5]=b[2]^(b[6]^b[7]);
        ao[6]=b[3]^b[7];
        ao[7]=b[4];
    }

    a=getByte(ao);
    return a;
}

void affTrans(int* a,int sel)
{
    int t[8]= {0};
    int A,B,C,D;

    for (int i=0; i<8; i++)
        t[i]=a[i];

    if(sel==0)
    {
        A=a[0]^a[1];
        B=a[2]^a[3];
        C=a[4]^a[5];
        D=a[6]^a[7];
        a[0] = (1^t[0]) ^ C ^ D;
        a[1] = (1^t[5]) ^ A ^ D;
        a[2] = (0^t[2]) ^ A ^ D;
        a[3] = (0^t[7]) ^ A ^ B;
        a[4] = (0^t[4]) ^ A ^ B;
        a[5] = (1^t[1]) ^ B ^ C;
        a[6] = (1^t[6]) ^ B ^ C;
        a[7] = (0^t[3]) ^ C ^ D;
    }
    else if(sel==-1)
    {
        A=a[0]^a[5];
        B=a[1]^a[4];
        C=a[2]^a[7];
        D=a[3]^a[6];
        a[0] = (1^t[5]) ^ C ;
        a[1] = (0^t[0]) ^ D ;
        a[2] = (1^t[7]) ^ B ;
        a[3] = (0^t[2]) ^ A ;
        a[4] = (0^t[1]) ^ D ;
        a[5] = (0^t[4]) ^ C ;
        a[6] = (0^t[3]) ^ A ;
        a[7] = (0^t[6]) ^ B ;
    }
}
void SubBytes(unsigned char state[][4],int mode)
{
    int r,c;

    for(r=0; r<4; r++)
    {
        for(c=0; c<4; c++)
        {
            state[r][c] = mode==0 ? sBox[state[r][c]] :invsBox[state[r][c]];
        }
    }
}

void KeyExpansion(const unsigned char* key, unsigned char w[][4][4],int mode)
{
    int i,j,r,c;
    //unsigned char rc[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
    int rcSft[8]       = {1,0,0,0,0,0,0,0};
    unsigned char rcOut[10];
    unsigned char rcOutInv[10];

    for(i=1; i<=10; i++)
    {
        int feedback=0;
        rcOut[i-1]=getByte(rcSft);
        feedback = rcSft[7];
        rcSft[7] = rcSft[6];
        rcSft[6] = rcSft[5];
        rcSft[5] = rcSft[4];
        rcSft[4] = rcSft[3]^feedback;
        rcSft[3] = rcSft[2]^feedback;
        rcSft[2] = rcSft[1];
        rcSft[1] = rcSft[0]^feedback;
        rcSft[0] = feedback;
        //printf("%2x ",rcOut[i-1]);
    }

    getBit(0x36,rcSft);

    for(i=1; i<=10; i++)
    {
        int feedback=0;
        rcOutInv[i-1]=getByte(rcSft);
        feedback = rcSft[0];
        rcSft[0] = rcSft[1]^feedback;
        rcSft[1] = rcSft[2];
        rcSft[2] = rcSft[3]^feedback;
        rcSft[3] = rcSft[4]^feedback;
        rcSft[4] = rcSft[5];
        rcSft[5] = rcSft[6];
        rcSft[6] = rcSft[7];
        rcSft[7] = feedback;
//        printf("%2x ",rcOutInv[i-1]);
    }

    unsigned char t[4];

    for(r=0; r<4; r++)
    {
        for(c=0; c<4; c++)
        {
            w[0][r][c] = key[r+c*4];
        }
    }

    for(i=1; i<=10; i++)
    {
        for(j=0; j<4; j++)
        {
            for(r=0; r<4; r++)
            {
                t[r] = j ? w[i][r][j-1] : w[i-1][r][3];
            }

            if(j == 0)
            {
                unsigned char temp = t[0];

                for(r=0; r<3; r++)
                {
                    t[r] = sBox[t[(r+1)%4]];
                }

                t[3] = sBox[temp];
                //t[0] ^= rc[i-1];
                t[0] ^= rcOut[i-1];
            }

            for(r=0; r<4; r++)
            {
                w[i][r][j] = w[i-1][r][j] ^ t[r];
            }
        }
    }

    if(mode==1)
    {
        for(int i=1; i<11; i++)
        {
            for(r=0; r<4; r++)
            {
                t[r]=w[10-i][r][3];
            }

            unsigned char temp = t[0];

            for(r=0; r<3; r++)
            {
                t[r] = sBox[t[(r+1)%4]];
            }

            t[3] = sBox[temp];
            //t[0] ^= rc[i-1];
            t[0] ^= rcOutInv[i-1];

            for(r=0; r<4; r++)
            {
                w[10-i][r][3]=w[10-i+1][r][3] ^ w[10-i+1][r][2];
                w[10-i][r][2]=w[10-i+1][r][2] ^ w[10-i+1][r][1];
                w[10-i][r][1]=w[10-i+1][r][1] ^ w[10-i+1][r][0];
                w[10-i][r][0]=w[10-i+1][r][0] ^ t[r];
            }
        }
    }
}

unsigned char FFmul(unsigned char a, unsigned char b)
{
    unsigned char bw[4];
    unsigned char res=0;
    int i;
    bw[0] = b;

    for(i=1; i<4; i++)
    {
        bw[i] = bw[i-1]<<1;

        if(bw[i-1]&0x80)
        {
            bw[i]^=0x1b;
        }
    }

    for(i=0; i<4; i++)
    {
        if((a>>i)&0x01)
        {
            res ^= bw[i];
        }
    }

    return res;
}


void ShiftRows(unsigned char state[][4],int mode)
{
    unsigned char t[4];
    int r,c;

    for(r=1; r<4; r++)
    {
        for(c=0; c<4; c++)
        {
            if(mode==0)
                t[c] = state[r][(c+r)%4];// --> 1 2 3
            else
                t[c] = state[r][(c+4-r)%4];//<--1 2 3
        }

        for(c=0; c<4; c++)
        {
            state[r][c] = t[c];
        }
    }
}

void MixColumns(unsigned char state[][4],int mode)
{
    unsigned char t[4];
    int  c;

    for(c=0; c< 4; c++)
    {
        t[0]=state[0][c];
        t[1]=state[1][c];
        t[2]=state[2][c];
        t[3]=state[3][c];
        state[0][c]= xtime(t[0]^t[1],1) ^ (t[2]^t[3]) ^ t[1];
        state[1][c]= xtime(t[1]^t[2],1) ^ (t[0]^t[3]) ^ t[2];
        state[2][c]= xtime(t[2]^t[3],1) ^ (t[0]^t[1]) ^ t[3];
        state[3][c]= xtime(t[0]^t[3],1) ^ (t[1]^t[2]) ^ t[0];

        if(mode==1)
        {
            state[0][c]= state[0][c] ^ xtime(t[0]^t[2],2) ^ xtime( (t[0]^t[2]) ^ (t[1]^t[3]),3);
            state[1][c]= state[1][c] ^ xtime(t[1]^t[3],2) ^ xtime( (t[0]^t[2]) ^ (t[1]^t[3]),3);
            state[2][c]= state[2][c] ^ xtime(t[0]^t[2],2) ^ xtime( (t[0]^t[2]) ^ (t[1]^t[3]),3);
            state[3][c]= state[3][c] ^ xtime(t[1]^t[3],2) ^ xtime( (t[0]^t[2]) ^ (t[1]^t[3]),3);
        }
    }
}

void AddRoundKey(unsigned char state[][4], unsigned char k[][4])
{
    int r,c;

    for(c=0; c<4; c++)
    {
        for(r=0; r<4; r++)
        {
            state[r][c] ^= k[r][c];
        }
    }
}

void printfStats(unsigned char state[][4],int mode)
{
#if(AES_DEBUG)
    int c,r;

    if(mode==1)
    {
        for(r=0; r<4; r++)
        {
            for(c=0; c<4; c++)
            {
                printf("%2x ",state[r][c]);
            }

            printf("\n");
        }

        printf("\n");
    }
#else
    (void) state;
    (void) mode;
#endif
}




void xor128bit(unsigned char* x,unsigned char* y,unsigned char* z)
{
    for(int i=0; i<16; i++)
    {
        z[i]=x[i]^y[i];
    }
}


extern int phy_sec_decrypt(const uint8_t* key, const uint8_t* iv,
                           uint8_t* din, uint32_t len, uint8_t* mic_in, uint8_t* dout);
extern int phy_sec_encrypt(const uint8_t* key, const uint8_t* iv,
                           uint8_t* din, uint32_t len, uint8_t* mic_out, uint8_t* dout);

extern uint32_t g_ota_sec_key[4];

typedef enum
{
    EFUSE_BLOCK_0 = 0,
    EFUSE_BLOCK_1 = 1,
    EFUSE_BLOCK_2 = 2,
    EFUSE_BLOCK_3 = 3,

} EFUSE_block_t;
//extern int efuse_lock(EFUSE_block_t block);

/****************************************************************************
    Function Name  : aes_ccm_phyplus_dec
    Description    : decrypt by phyplus defined aes ccm mode
    Input          :
                 : *iv,  13 byte
                 : *din, dLen byte
                 : dLen, input data length, exclude mic
                 : *mic, 4 byte, Message Identify Code
    Output         : (dout,dLen byte
    Return         : bool, according to the MIC check result
****************************************************************************/
#if 0
bool aes_ccm_phyplus_dec(const unsigned char* key,const unsigned char* iv, unsigned char* din,int dLen, unsigned char* micIn,
                         unsigned char* dout)
{
    //unsigned char key_o[16];
    int i = 0, j = 0;
    unsigned char ax[16];
    unsigned char bx[16];
    unsigned char y[16];
    unsigned char s[16];
    unsigned char ti[16];
    unsigned char to[16];
    unsigned char dummy_out[16];
    int loopNum = (dLen+15)/16;
    int resLen  = dLen-(loopNum-1)*16;
    unsigned short cnt=0;
    ax[0]=0x02;
    bx[0]=0x62;

    for(i =0; i<13; i++)
    {
        ax[i+1]=iv[i];
        bx[i+1]=iv[i];
    }

    bx[14]=((dLen/16)&0xff00)>>8;
    bx[15]=(dLen/16)&0xff;

    for(i=0; i<loopNum; i++)
    {
        unsigned char* pout = (dout == NULL) ? dummy_out : (dout+i*16);

        if(i== 0x9cf)
        {
            j ++;
        }

        //update counter in ccm
        cnt+=13;
        ax[14]=(cnt&0xff00)>>8;
        ax[15]=cnt&0x00ff;
        //aes((const unsigned char*)s_company_id,key_o,ax,s,AES_ENC);
        LL_ENC_AES128_Encrypt_X((unsigned char*)key,ax,s);

        if(i<loopNum-1)
        {
            xor128bit(s,din+i*16,pout);
        }
        //process for the last16byte
        else
        {
            for(int j=0; j<resLen; j++)
                ti[j]=din[i*16+j];

            for(int j=resLen; j<16; j++)
                ti[j]=0x00;

            xor128bit(s,ti,to);

            for(int j=0; j<resLen; j++)
                pout[j]=to[j];

            for(int j=resLen; j<16; j++)
                to[j]=0x00;
        }

        //cbc
        //aes((const unsigned char*)s_company_id,key_o,bx,y,AES_ENC);
        LL_ENC_AES128_Encrypt_X((unsigned char*)key,bx,y);

        if(i<loopNum-1)
        {
            xor128bit(y,pout,bx);
        }
        else
        {
            xor128bit(y,to,bx);
        }
    }

    //check mic
    //aes((const unsigned char*)s_company_id,key_o,bx,y,AES_ENC);
    LL_ENC_AES128_Encrypt_X((unsigned char*)key,bx,y);
    ax[14]=0;
    ax[15]=0;
    //aes((const unsigned char*)s_company_id,key_o,ax,s,AES_ENC);
    LL_ENC_AES128_Encrypt_X((unsigned char*)key,ax,s);

    if(micIn == NULL)
        return true;

    for(i=0; i<4; i++)
    {
        if((y[i]^s[i])!=micIn[i])
        {
            return false;
        }
    }

    return true;
}
#endif


bool is_crypto_app(void)
{
    //uint8_t key[16];
    if(finidv() == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

int flash_load_parition(unsigned char* pflash, int size, unsigned char* micIn,unsigned char* run_addr)
{
    return phy_sec_decrypt((unsigned char*)g_ota_sec_key,(unsigned char*)0x11002830, pflash, size, micIn, run_addr);
}

int flash_check_parition(unsigned char* pflash, int size, unsigned char* run_addr,unsigned char* micOut)
{
    return phy_sec_encrypt((unsigned char*)g_ota_sec_key,(unsigned char*)0x11002830,pflash,size,run_addr,micOut);
}



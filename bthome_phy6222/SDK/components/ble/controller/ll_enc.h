/*******************************************************************************
  Filename:       ll_enc.h
  Revised:         
  Revision:        

  Description:    This file contains the Link Layer (LL) types, contants,
                  API's etc. for the Bluetooth Low Energy (BLE) Controller
                  CCM encryption and decryption.

                  This API is based on ULP BT LE D09R23.

  SDK_LICENSE

*******************************************************************************/

#ifndef LL_ENC_H
#define LL_ENC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "ll_def.h"    

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#define LL_ENC_TX_DIRECTION_MASTER   1
#define LL_ENC_TX_DIRECTION_SLAVE    0
#define LL_ENC_RX_DIRECTION_MASTER   0
#define LL_ENC_RX_DIRECTION_SLAVE    1
  
#define LL_ENC_DATA_BANK_MASK 0xFF7F

#define LL_ENC_TRUE_RAND_BUF_SIZE     ((LL_ENC_IV_LEN/2) + (LL_ENC_SKD_LEN/2))

// Generate Session Key using LTK for key and SKD for plaintext.
#define LL_ENC_GenerateSK LL_ENC_AES128_Encrypt

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
extern uint8 dataPkt[2*LL_ENC_BLOCK_LEN];
extern uint8 cachedTRNGdata[ LL_ENC_TRUE_RAND_BUF_SIZE ];

/*******************************************************************************
 * Functions
 */

// Random Number Generation
extern uint8 LL_ENC_GeneratePseudoRandNum( void );
extern uint8 LL_ENC_GenerateTrueRandNum( uint8 *buf, uint8 len );

// CCM Encryption
extern void  LL_ENC_AES128_Encrypt( uint8 *key, uint8 *plaintext,  uint8 *ciphertext );
extern void  LL_ENC_AES128_Decrypt( uint8 *key, uint8 *ciphertext, uint8 *plaintext );
extern void  LL_ENC_LoadEmptyIV( void );
extern void  LL_ENC_ReverseBytes( uint8 *buf, uint8 len );
extern void  LL_ENC_GenDeviceSKD( uint8 *SKD );
extern void  LL_ENC_GenDeviceIV( uint8 *IV );
extern void  LL_ENC_GenerateNonce( uint32 pktCnt, uint8 direction, uint8 *nonce );
extern void  LL_ENC_EncryptMsg( uint8 *nonce, uint8 pktLen, uint8 *pbuf, uint8 *mic );
extern void  LL_ENC_DecryptMsg( uint8 *nonce, uint8 pktLen, uint8 *pBuf, uint8 *mic );
extern void  LL_ENC_Encrypt( llConnState_t *connPtr, uint8 pktHdr, uint8 pktLen, uint8 *pBuf );
extern uint8 LL_ENC_Decrypt( llConnState_t *connPtr, uint8 pktHdr, uint8 pktLen, uint8 *pBuf );
extern void LL_ENC_sm_ah( uint8 *pK, uint8 *pR, uint8 *pAh );
//

extern void  LL_ENC_MoveData( uint8 *pDst, uint8 *pSrc, uint16 len );

#ifdef __cplusplus
}
#endif

#endif /* LL_ENC_H */

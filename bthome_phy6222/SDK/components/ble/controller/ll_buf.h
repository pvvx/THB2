/*************
 ll_buf.h
 SDK_LICENSE
***************/

#ifndef _LL_BUF_H_
#define _LL_BUF_H_
#include <stdint.h>

#define MAX_ADV_BUF            1                   // buffer advertisement packet
#define TYP_CONN_BUF_LEN       4                   // typical packet number per connection event
#define MAX_LL_BUF_LEN         8                   // maximum LL buffer for Rx/Tx packet


//#define LL_CTRL_PDU_LEN           29          //V4.0/4.2: 2octets header + 27 octets payload
//#define LL_ADV_PDU_LEN            39          //V4.0/4.2: 2octets header + 37 octets payload
//#define LL_DATA_PDU_LEN           33          //V4.0    : 2octets header + 31 octets payload

// for Rx FIFO, HW will pack zero bytes to align 4bytes boarder
// BLE 4.0, PDU length < 39, 3Bytes CRC will also be read.
// note that PDU head is write in "rxheader/txheader" field, the buffer need (39 + 3 - 2) = 40 bytes, 
// add 2 bytes to align 4 bytes edge
//#define BLE_PACKET_BUF_LEN        42
//257+3-2=256
// BLE5.0, PDU length: maximum 257 octets, 3 octets CRC, 2 octets PDU head is write in "rxheader/txheader" field
//#define BLE_PACKET_BUF_LEN        258  //(257+3-2) 

// BLE 5.1, PDU length: 2-258 octets, 3 octets CRC, 2 octets PDU head is write in "rxheader/txheader" field
// length should align to word edge, so + 3 octet
#define BLE_PACKET_BUF_LEN        262  //(258+3-2) + 3


#define RX_BUF_LEN                BLE_PACKET_BUF_LEN          
#define TX_BUF_LEN                BLE_PACKET_BUF_LEN
#define TX_CTRL_BUF_LEN           34 //(27+4+3)

#define LL_PDU_LENGTH_SUPPORTED_MAX_TX_OCTECTS          251
#define LL_PDU_LENGTH_SUPPORTED_MAX_RX_OCTECTS          251
#define LL_PDU_LENGTH_SUPPORTED_MAX_TX_TIME             2120
#define LL_PDU_LENGTH_SUPPORTED_MAX_RX_TIME             2120

#define LL_PDU_LENGTH_INITIAL_MAX_TX_OCTECTS            27
#define LL_PDU_LENGTH_INITIAL_MAX_RX_OCTECTS            27
#define LL_PDU_LENGTH_INITIAL_MAX_TX_TIME               328
#define LL_PDU_LENGTH_INITIAL_MAX_RX_TIME               328

// BBB update
struct ll_pkt_desc
{
    uint32_t    valid;  // mean a valid data received from ble
    uint16_t    header;
    uint8_t     data[2];   
};


struct buf_rx_desc
{
    uint32_t    valid;  // mean a valid data received from ble
	/// rx header
    uint16_t rxheader;
    uint8_t data[RX_BUF_LEN ];   // for v4.2 BLE, set to 256
};

struct buf_tx_desc
{
    uint32_t  valid; // means a valid data to wait for send to ble
    //uint32_t  sent;  // means tha data has been sent before
    /// tx header
    uint16_t txheader;
    /// data
    uint8_t data[TX_BUF_LEN];
};

typedef struct
{
	uint16_t header;
	//uint8_t  data[TX_BUF_LEN];
	uint8_t  data[TX_CTRL_BUF_LEN];
} __attribute__((aligned(4))) ctrl_packet_buf;

typedef struct   
{ 
#if 0
    struct buf_tx_desc tx_conn_desc[MAX_LL_BUF_LEN];     // new Tx data buffer
    struct buf_rx_desc rx_conn_desc[MAX_LL_BUF_LEN];

    struct buf_tx_desc tx_not_ack_pkt;
    struct buf_tx_desc tx_ntrm_pkts[MAX_LL_BUF_LEN];    
#endif
    struct ll_pkt_desc *tx_conn_desc[MAX_LL_BUF_LEN];     // new Tx data buffer
    struct ll_pkt_desc *rx_conn_desc[MAX_LL_BUF_LEN];

    struct ll_pkt_desc *tx_not_ack_pkt;
    struct ll_pkt_desc *tx_ntrm_pkts[MAX_LL_BUF_LEN];    

    
    uint8_t ntrm_cnt;       // number of packets not transmit
    
    uint8_t tx_write;
    uint8_t tx_read;
    uint8_t tx_loop;        // flag for write ptr & read ptr work in the same virtual buffer bank

    uint8_t rx_write;
    uint8_t rx_read;    
    uint8_t rx_loop;        // flag for write ptr & read ptr work in the same virtual buffer bank 	  
} llLinkBuf_t;



#endif

/*******************************************************************************
    @file     i2c_comman.h
    @brief    i2c General Configuration
    @version  1.0

 SDK_LICENSE

*******************************************************************************/

#ifndef __I2C_COMMON_H__
#define __I2C_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
    @ Module           : Includes
    @ Description    :  NULL
*******************************************************************************/
#include "bus_dev.h"
#include "gpio.h"

/*******************************************************************************
    @ Module           :  pre-compile
    @ Description    :  NULL
*******************************************************************************/


/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  IIC Error Code
*******************************************************************************/
#define PPlus_IIC_SUCCESS                           0
#define PPlus_ERR_IIC_ADDRESS               0xFE
#define PPlus_ERR_IIC_ID                            0xFD
#define PPLUS_ERR_IIC_BUSY                      0xFC
#define PPLUS_ERR_IIC_ENABLE                    0xFB
#define PPlus_ERR_IIC_FAILURE                   0xFF

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  IIC InValid Handle
*******************************************************************************/
#define PPlus_INVALID_HANDLE                    0xFF

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  The number of IIC Module(Two IIC:IIC0,IIC1)
*******************************************************************************/
#define IIC_COUNT   2

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  Indicate IIC Module
*******************************************************************************/
#define IIC_Module0         0
#define IIC_Module1         1

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  IIC Bus Read or Wirte Device
*******************************************************************************/
#define I2C_COMMAND_READ        0x01
#define I2C_COMMAND_WRITE       0x00

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  IIC Bus Read or Write data from FIFO
*******************************************************************************/
#define I2C_Data_READ               0x01
#define I2C_Data_WRITE          0x00

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  Can not be used IIC Slave Address Length
*******************************************************************************/
#define I2C_NOT_USED_SLAVE_ADDRLEN  4

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  None
*******************************************************************************/
#define I2C_IC_DEFAULT_TAR_UPDATE           0
#define I2C_IC_CLK_TYPE                             1

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  IIC Default Master/Slave Address
*******************************************************************************/
#define I2C_IC_DEFAULT_ADDR 0x0010

/*******************************************************************************
    @ Module           :  I2C Buffer Length
    @ Description    :  relate to RX FIFO & TX FIFO
*******************************************************************************/
#define I2C_FIFO_DEPTH          8
#define I2C_RX_TL_CNT               I2C_FIFO_DEPTH
#define I2C_TX_TL_CNT               I2C_FIFO_DEPTH

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  IC_CON Bit Field Description
*******************************************************************************/
// Determine IIC Slave enable or disable after reset(salve is enable in reset state)
// if slave enabled , MASTER_MODE should be disabled
#define IC_CON_IC_SLAVE_DISABLE     0x0040

// Determine whether restart conditions may be send when act as a master
// when act an a master , must enable this function
#define IC_CON_IC_RESTART_EN        0x0020

#define IC_CON_IC_10BITADDR_MASTER  0x0010
// Determine the IIC Master Address Mode(10bit)

// Determine the IIC Slave Address Mode(10bit)
#define IC_CON_IC_10BITADDR_SLAVE   0x0008

// Determine IIC Speed Mode
#define IC_CON_IC_SPEED_100K        0x0002
#define IC_CON_IC_SPEED_400K        0x0004
#define IC_CON_IC_SPEED_3_4M        0x0006

// Determine IIC Master Enable
#define IC_CON_MASTER_MODE          0x0001

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  IC_TAR Bit Field Description
*******************************************************************************/
// this bit related to I2C_IC_DEFAULT_TAR_UPDATE Value
#define IC_TAR_IC_10BITADDR_MASTER  0x1000

// Indicate whether performance a Gengral call or start byte Command
// relate to IC_TAR_GC_OR_START
#define IC_TAR_SPECIAL_COMMAND_ENABLE   0x0800

// Dependency IC_TAR_SPECIAL_COMMAND_ENABLE
// if set, Start Byte,otherwise General Call
// General Call:only writes may be performed.when read resault TX_ABRT
// Start Byte:send Start Byte(0x01) to Synchronizate IIC Clock
#define IC_TAR_GC_OR_START              0x0400

// IC Default Slave Address
#define IC_TAR_DEFAULT_SLAVE_ADDRESS    I2C_IC_DEFAULT_TAR_SLAVE_ADDR

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  IC_SAR
*******************************************************************************/
#define IC_SAR_DEFAUT_SLAVE_ADDRESS     I2C_IC_DEFAULT_TAR_SLAVE_ADDR

/*******************************************************************************
    @ Module           :  I2C Interrupt Status Register
    @ Description    :  Interrupt Status bit(cleared by reading the matching interrupt clear register.)
*******************************************************************************/
#define I2C_INTR_STAT_RX_UNDER          0x0001
#define I2C_INTR_STAT_RX_OVER           0x0002
#define I2C_INTR_STAT_RX_FULL           0x0004
#define I2C_INTR_STAT_TX_OVER           0x0008
#define I2C_INTR_STAT_TX_EMPTY          0x0010
#define I2C_INTR_STAT_RD_REQ                0x0020
#define I2C_INTR_STAT_TX_ABRT           0x0040
#define I2C_INTR_STAT_RX_DONE           0x0080
#define I2C_INTR_STAT_ACTIVITY          0x0100
#define I2C_INTR_STAT_STOP_DET          0x0200
#define I2C_INTR_STAT_START_DET         0x0400
#define I2C_INTR_STAT_GEN_CALL          0x0800

/*******************************************************************************
    @ Module           :  I2C Interrupt Mask Register
    @ Description    :  Interrupt MASK bit
*******************************************************************************/
#define I2C_MASK_RX_UNDER           0x0001
#define I2C_MASK_RX_OVER            0x0002
#define I2C_MASK_RX_FULL            0x0004
#define I2C_MASK_TX_OVER            0x0008
#define I2C_MASK_TX_EMPTY           0x0010
#define I2C_MASK_RD_REQ             0x0020
#define I2C_MASK_TX_ABRT            0x0040
#define I2C_MASK_RX_DONE            0x0080
#define I2C_MASK_ACTIVITY           0x0100
#define I2C_MASK_STOP_DET           0x0200
#define I2C_MASK_START_DET      0x0400
#define I2C_MASK_GEN_CALL           0x0800
#define I2C_DISABLE_ALL_INTR    0

/*******************************************************************************
    @ Module           :  I2C Raw Interrupt Register
    @ Description    :  Interrupt MASK bit
*******************************************************************************/
#define I2C_RAW_INTR_RX_UNDER           0x0001
#define I2C_RAW_INTR_RX_OVER            0x0002
#define I2C_RAW_INTR_RX_FULL            0x0004
#define I2C_RAW_INTR_TX_OVER            0x0008
#define I2C_RAW_INTR_TX_EMPTY           0x0010
#define I2C_RAW_INTR_RD_REQ             0x0020
#define I2C_RAW_INTR_TX_ABRT            0x0040
#define I2C_RAW_INTR_RX_DONE            0x0080
#define I2C_RAW_INTR_ACTIVITY           0x0100
#define I2C_RAW_INTR_STOP_DET           0x0200
#define I2C_RAW_INTR_START_DET      0x0400
#define I2C_RAW_INTR_GEN_CALL           0x0800

/*******************************************************************************
    @ Module           :  I2C ENABLE Status Register
    @ Description    :  I2C ENABLE STATUS Bit
*******************************************************************************/
#define I2C_ENSTA_IC_ENABLE_STATUS      0x0001
#define I2C_ENSTA_SLV_RX_ABORT          0x0002
#define I2C_ENSTA_SLV_RX_DATA_LOST      0x0004

/*******************************************************************************
    @ Module           :  I2C Status Register
    @ Description    :  Only read register ,indicate current transfer stauts and FIFO status
*******************************************************************************/
#define I2C_STAT_SLV_ACTIVITY           0x0040
#define I2C_STAT_MST_ACTIVITY           0x0020
//Receive FIFO Completely Full
#define I2C_STAT_RFF                    0x0010
//Receive FIFO Not Empty
#define I2C_STAT_RFNF                   0x0008
// Transmit FIFO Completely Empty
#define I2C_STAT_TFE                    0x0004
// Transmit FIFO Not Full
#define I2C_STAT_TFNF                   0x0002
#define I2C_STAT_ACTIVITY               0x0001

/*******************************************************************************
    @ Module           :  I2C Transmit Abort Source Register
    @ Description    :   indicate the source of the TX_ABRT
*******************************************************************************/
#define I2C_ABRT_7B_ADDR_NOACK          0x0001
#define I2C_ABRT_10B_ADDR1_NOACK        0x0002
#define I2C_ABRT_10B_ADDR2_NOACK        0x0004
#define I2C_ABRT_TXDATA_NOACK           0x0008
// master send a General Call , but no slave ACK
#define I2C_ABRT_GCALL_NOACK            0x0010
// master send data to the bus , but user programmed the data direction is read
// from the bus(relate to IC_DATA_CMD[8])
#define I2C_ABRT_GCALL_READ             0x0020
// master in high speed mode ,and its code was ACK(Should not ACK)
#define I2C_ABRT_HS_ACKDET              0x0040
// master send START Byte and detect ACK(Should not ACK)
#define I2C_ABRT_SBYTE_ACKDET           0x0080
// the restart is diabled,and the user is trying to use the master to transmit
// data in high speed mode
#define I2C_ABRT_HS_NORTART             0x0100

//
#define I2C_ABRT_SBYTE_NORESTART        0x0200

#define I2C_ABRT_10B_RD_NORSTART        0x0400
// user tries to initiate a master operation with the master mode disabled
#define I2C_ABRT_MASTER_DIS             0x0800

#define I2C_ABR_LOST                    0x1000

// slave receive a read command ,but there's some data exists in TX FIFO,
// and issues interrupt to flush old TX FIFO
#define I2C_ABRT_SLVFLUSH_TXFIFO        0x2000

#define I2C_ABRT_SLV_ABRLOST            0x4000

// slave transmit data to the master , and use the read data command from the
// bus(relate to IC_DATA_CMD[8])
#define I2C_ABRT_SLVRD_INTX             0x8000

#define I2C0_IRQ I2C0_IRQn
#define I2C1_IRQ I2C1_IRQn
/*******************************************************************************
    @ Module               :  IIC Work Mode
    @ Description    :  None
*******************************************************************************/
typedef enum
{
    Slave = 0,
    Master = IC_CON_IC_SLAVE_DISABLE | IC_CON_IC_RESTART_EN | IC_CON_MASTER_MODE,
} I2C_WorkMode;

/*******************************************************************************
    @ Module               :  IIC Speed Mode
    @ Description    :  None
*******************************************************************************/
typedef enum
{
    SPEED_STANDARD = 0x0002,               //standard mode
    SPEED_FAST = 0x0004,                   //fast mode
    SPEED_HIGH = 0x0006,                   //high mode
} I2C_SPEED_e;

/*******************************************************************************
    @ Module               :  IIC Clock
    @ Description    :  None
*******************************************************************************/
typedef enum
{
    I2C_CLOCK_100K  = 0x0002,
    I2C_CLOCK_400K  = 0x0004,
    I2C_CLOCK_3_4M  = 0x0006,
} I2C_CLOCK_e;

/*******************************************************************************
    @ Module               :  IIC Address Mode
    @ Description    :  Only support 7 bit Address,be careful while using
*******************************************************************************/
typedef enum
{
    I2C_ADDR_7bit = 0,
    I2C_ADDR_10bit
} I2C_ADDRESS_e;

/*******************************************************************************
    @ Module               :  transfer mode
    @ Description    :
                    I2C_MODE_BLOCKING blocks task execution while an I2C transfer is in progress
                    I2C_MODE_CALLBACK does not block task execution; but calls a callback
                    function when the I2C transfer has completed
*******************************************************************************/
typedef enum
{
    I2C_MODE_BLOCKING,  /*!< I2C_transfer blocks execution*/
    I2C_MODE_CALLBACK   /*!< I2C_transfer queues transactions and does not block */
} I2C_TransferMode;

/*******************************************************************************
    @ Module               :   I2C Event enum
    @ Description    :   None
*******************************************************************************/
typedef enum
{
    // INTR SOURCE EVENT
    I2C_RX_UNDER_Evt = 0x0001,
    I2C_RX_OVER_Evt = 0x0002,
    I2C_RX_FULL_Evt = 0x0004,
    I2C_TX_OVER_Evt = 0x0008,
    I2C_TX_EMPTY_Evt = 0x0010,
    I2C_RD_REQ_Evt = 0x0020,
    I2C_TX_ABRT_Evt = 0x0040,
    I2C_RX_DONE_Evt = 0x0080,
    I2C_ACTIVITY_Evt = 0x0100,
    I2C_STOP_DET_Evt = 0x0200,
    I2C_START_DET_Evt = 0x0400,
    I2C_GEN_CALL_Evt = 0x0800,
    // User Event
    I2C_DINIT_SUCCESS = 0x1000
} I2C_EVT;

/*******************************************************************************
    @ Module               :   I2C Event
    @ Description    :   None
*******************************************************************************/
typedef struct
{
    I2C_EVT  type;
    uint8_t   len;
} I2C_Evt_t;

/*******************************************************************************
    @ Module               :   Function *p
    @ Description    :   None
*******************************************************************************/
typedef void (*I2C_Hdl_t)(I2C_Evt_t* pev);



/*******************************************************************************
    @ Module               :   I2C Master Paramter structure
    @ Description    :   None
*******************************************************************************/
typedef struct
{
    // General
    uint8_t                     id;                                         // set
    I2C_WorkMode            workmode;                               // init some Para ,according to workmode
    I2C_CLOCK_e             ClockMode;
    I2C_ADDRESS_e       AddressMode;
    bool                    use_fifo;                               // check if use fifo,default used
    uint8_t                     RX_FIFO_Len;                        // RX , TX FIFO SET
    uint8_t                     Tx_FIFO_Len;
    gpio_pin_e              SDA_PIN;                                // I2C Pin
    gpio_pin_e              SCL_PIN;
    I2C_TransferMode    TransferMode;                       // according to TransferMode, that evt_handler can be
    // enable or disable
    I2C_Hdl_t               evt_handler;

    uint8_t                     Master_Addressing;          // TAR,when as master
    uint32_t                    IC_xS_SCL_HCNT;
    uint32_t                    IC_xS_SCL_LCNT;

} I2C_Master_Parameter;

/*******************************************************************************
    @ Module           :  Master Default Init Parameter
    @ Description    :   None
*******************************************************************************/
//I2CCOM_Ext I2C_Parameter I2C_DefaultPara;

/*******************************************************************************
    @ Module               :  Function statement
    @ Description    :
*******************************************************************************/
uint8_t Hal_Check_IIC_IsAlready_Closed(void);

/*******************************************************************************
    @ Module               :  Get_IIC_Instance
    @ Description    :
                    [para in]:id,iic module id 0 or 1
                    [return]:instance address
*******************************************************************************/
AP_I2C_TypeDef* Hal_Get_IIC_Instance(uint8_t id);

/*******************************************************************************
    @ Module               :  Hal_GetIIC_ModuleID
    @ Description    :  None
*******************************************************************************/
MODULE_e Hal_GetIIC_ModuleID(uint8_t id);

/*******************************************************************************
    @ Module               :  Hal_GetIIC_Interrupt ID
    @ Description    :  None
*******************************************************************************/
uint8_t Hal_GetIIC_IRQID(uint8_t id);

/*******************************************************************************
    @ Module               :  Hal_GetIIC_PIN_Fmux ID
    @ Description    :  None
*******************************************************************************/
void Hal_GetIIC_PIN_Fmux(uint8_t id,Fmux_Type_e* SCL_Fmux,Fmux_Type_e* SDA_Fmux);

/*******************************************************************************
    @ Module               :  IIC Register CallBack Function
    @ Description    :
                    [para in]:id,iic module 0,1;cb , call back function
*******************************************************************************/
uint8_t Hal_IIC_Register_CallBack(uint8_t id,I2C_Hdl_t cb);

/*******************************************************************************
    @ Module               :  IIC unRegister CallBack Function
    @ Description    :  [para in]:id,iic module 0,1
*******************************************************************************/
uint8_t Hal_IIC_unRegister_CallBack(uint8_t id);

/*******************************************************************************
    @ Module               :  IIC Check IIC Valid
    @ Description    :  [para in]:id,iic module 0,1
*******************************************************************************/
uint8_t Hal_IIC_Valid_Check(uint8_t id);

/*******************************************************************************
    @ Module               :  IIC Check IIC Address Valid
    @ Description    :  [para in]:Addr,usr set master or slave address
*******************************************************************************/
uint8_t Hal_IIC_Addr_Valid(uint8_t Addr);

/*******************************************************************************
    @ Module               :  Check IIC Closed Success
    @ Description    :  [para in]:Ins,the instance wanted to check
*******************************************************************************/
uint8_t Hal_Check_IIC_Closed(AP_I2C_TypeDef* Ins);

/*******************************************************************************
    @ Module               :  IIC Receive data from RX FIFO
    @ Description    :  [para in]:Ins,the instance wanted to READ Data
*******************************************************************************/
uint8_t Hal_IIC_Read_RXFIFO(AP_I2C_TypeDef* Ins);

/*******************************************************************************
    @ Module               :  Hal_INTR_SOURCE_Clear
    @ Description    :  [para in]:Ins,the instance wanted to READ Data,irqs:interrupt source
*******************************************************************************/
void Hal_INTR_SOURCE_Clear(AP_I2C_TypeDef* Ins,uint32_t irqs);

/*******************************************************************************
    @ Module               :  Hal_IIC_Write_TXFIFO
    @ Description    :  [para in]:Ins,the instance wanted to write,
*******************************************************************************/
void Hal_IIC_Write_TXFIFO(AP_I2C_TypeDef* Ins,uint8_t data);



void Hal_TRANS_ABRT_SourceCheck(void);

void __attribute__((weak)) Hal_I2C0_IRQHandler(void);
void __attribute__((weak)) Hal_I2C1_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif

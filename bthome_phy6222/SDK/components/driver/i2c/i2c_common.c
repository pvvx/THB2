/*******************************************************************************
    @file     i2c_comman.c
    @brief    i2c general Function
    @version  1.0

 SDK_LICENSE
 
*******************************************************************************/

/*******************************************************************************
    @ Module               :  Pre-Define
    @ Description    :  None
*******************************************************************************/
#define _I2C_Common_

/*******************************************************************************
    @ Module               :  Includes
    @ Description    :  None
*******************************************************************************/
#include "i2c_common.h"
#include "error.h"
#include "log.h"

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  None
*******************************************************************************/
#define CONCAT_2(p1,p2)             p1##p2
#define CONCAT_3(p1,p2,p3)      p1##p2##p3
#define DRV_IIC_INSTANCE(id)        CONCAT_2(AP_I2C,id)
#define DRV_GET_IIC_MOD_ID(id)  CONCAT_2(MOD_I2C,id)
#define DRV_GET_IIC_IRQ_ID(id)  CONCAT_3(I2C,id,_IRQ)

/*******************************************************************************
    @ Module               :  Typedef struct
    @ Description    :  I2C Module
*******************************************************************************/
typedef struct
{
    uint8_t                     busy;                                       // busy
} I2C_Mode_t;


/*******************************************************************************
    @ Module               :  Internal Variable
    @ Description    :  None
*******************************************************************************/
static I2C_Mode_t I2C_Mode[IIC_COUNT];

/*******************************************************************************
    @ Module               :  I2C IRQ Handler
    @ Description    :  None
*******************************************************************************/
I2C_Hdl_t I2C0_IRQ_Handle = NULL;
I2C_Hdl_t I2C1_IRQ_Handle = NULL;

/*******************************************************************************
    @ Module               :  IIC Slave Address pre-Declare
    @ Description    :  address , which can not be used
*******************************************************************************/
const unsigned char I2C_INVALID_SLAVE_ADDR[I2C_NOT_USED_SLAVE_ADDRLEN] = {0x00,0x07,0x78,0x7f};


/*******************************************************************************
    @ Module               :  Get_IIC_Instance
    @ Description    :  None
*******************************************************************************/
AP_I2C_TypeDef* Hal_Get_IIC_Instance(uint8_t id)
{
    if(IIC_Module0 == id)
        return (((AP_I2C_TypeDef*)DRV_IIC_INSTANCE(IIC_Module0)));
    else
        return (((AP_I2C_TypeDef*)DRV_IIC_INSTANCE(IIC_Module1)));
}

/*******************************************************************************
    @ Module               :  Hal_GetIIC_ModuleID
    @ Description    :  Get IIC Module Index as MODULE_e type
*******************************************************************************/
MODULE_e Hal_GetIIC_ModuleID(uint8_t id)
{
    if(IIC_Module0 == id)
        return DRV_GET_IIC_MOD_ID(IIC_Module0);
    else
        return DRV_GET_IIC_MOD_ID(IIC_Module1);
}

/*******************************************************************************
    @ Module               :  Hal_GetIIC_IRQID
    @ Description    :  Get IIC Interrupt Index
*******************************************************************************/
uint8_t Hal_GetIIC_IRQID(uint8_t id)
{
    if(IIC_Module0 == id)
        return DRV_GET_IIC_IRQ_ID(IIC_Module0);
    else
        return DRV_GET_IIC_IRQ_ID(IIC_Module1);
}

/*******************************************************************************
    @ Module               :  Hal_GetIIC_PIN_Fmux
    @ Description    :  None
*******************************************************************************/
void Hal_GetIIC_PIN_Fmux(uint8_t id,Fmux_Type_e* SCL_Fmux,Fmux_Type_e* SDA_Fmux)
{
    if(IIC_Module0 == id)
    {
        *SCL_Fmux = FMUX_IIC0_SCL;
        *SDA_Fmux = FMUX_IIC0_SDA;
    }
    else
    {
        *SCL_Fmux = FMUX_IIC1_SCL;
        *SDA_Fmux = FMUX_IIC1_SDA;
    }
}

/*******************************************************************************
    @ Module               :  Register call back function
    @ Description    :  None
*******************************************************************************/
uint8_t Hal_IIC_Register_CallBack(uint8_t id,I2C_Hdl_t cb)
{
    if(id == IIC_Module0)
    {
        I2C_Mode[IIC_Module0].busy = TRUE;
        I2C0_IRQ_Handle = cb;
        return PPlus_IIC_SUCCESS;
    }
    else if( id == IIC_Module1 )
    {
        I2C_Mode[IIC_Module1].busy = TRUE;
        I2C1_IRQ_Handle = cb;
        return PPlus_IIC_SUCCESS;
    }

    return PPlus_ERR_IIC_ID;
}

/*******************************************************************************
    @ Module               :  unregister call back function
    @ Description    :  None
*******************************************************************************/
uint8_t Hal_IIC_unRegister_CallBack(uint8_t id)
{
    if(id == IIC_Module0)
    {
        I2C_Mode[IIC_Module0].busy = FALSE;
        I2C0_IRQ_Handle = NULL;
        return PPlus_IIC_SUCCESS;
    }
    else if( id == IIC_Module1 )
    {
        I2C_Mode[IIC_Module1].busy = FALSE;
        I2C1_IRQ_Handle = NULL;
        return PPlus_IIC_SUCCESS;
    }

    return PPlus_ERR_IIC_ID;
}

/*******************************************************************************
    @ Module               :  IIC Check IIC Valid
    @ Description    :  [para in]:id,iic module 0,1
*******************************************************************************/
uint8_t Hal_IIC_Valid_Check(uint8_t id)
{
    if( id > (IIC_COUNT-1))
        return PPlus_ERR_IIC_ID;
    else
        return (I2C_Mode[id].busy);
}

/*******************************************************************************
    @ Module               :  IIC Check IIC Address Valid
    @ Description    :  [para in]:Addr,usr set master or slave address
*******************************************************************************/
uint8_t Hal_IIC_Addr_Valid(uint8_t Addr)
{
    for(unsigned char i=0; i<I2C_NOT_USED_SLAVE_ADDRLEN; i++)
    {
        if(I2C_INVALID_SLAVE_ADDR[i] == Addr)
            return PPlus_ERR_IIC_ADDRESS;
    }

    return PPlus_IIC_SUCCESS;
}

/*******************************************************************************
    @ Module               :  Check IIC Closed Success
    @ Description    :  [para in]:Ins,the instance wanted to check
*******************************************************************************/
uint8_t Hal_Check_IIC_Closed(AP_I2C_TypeDef* Ins)
{
    if(Ins->IC_ENABLE_STATUS & 0x0001)
    {
        // IIC Already enabled
        return PPLUS_ERR_IIC_ENABLE;
    }
    else
    {
        // IIC Closed Successed
        return PPlus_IIC_SUCCESS;
    }
}

/*******************************************************************************
    @ Module               :  IIC Receive data from RX FIFO
    @ Description    :  [para in]:Ins,the instance wanted to READ Data
*******************************************************************************/
uint8_t Hal_IIC_Read_RXFIFO(AP_I2C_TypeDef* Ins)
{
    return Ins->IC_DATA_CMD ;
}

/*******************************************************************************
    @ Module               :  Hal_INTR_SOURCE_Clear
    @ Description    :  [para in]:Ins,the instance wanted to READ Data,irqs:interrupt source
*******************************************************************************/
void Hal_INTR_SOURCE_Clear(AP_I2C_TypeDef* Ins,uint32_t irqs)
{
    switch( irqs )
    {
    case I2C_MASK_RX_UNDER:
        Ins->IC_CLR_UNDER;
        break;

    case I2C_MASK_RX_OVER:
        Ins->IC_CLR_RX_OVER;
        break;

    case I2C_MASK_TX_OVER:
        Ins->IC_CLR_TX_OVER;
        break;

    case I2C_MASK_RD_REQ:
        Ins->IC_CLR_RD_REG;
        break;

    case I2C_MASK_TX_ABRT:
        Ins->IC_CLR_TX_ABRT;
        break;

    case I2C_MASK_RX_DONE:
        Ins->IC_CLR_RX_DONE;
        break;

    case I2C_MASK_ACTIVITY:
        Ins->IC_CLR_ACTIVITY;
        break;

    case I2C_MASK_STOP_DET:
        Ins->IC_CLR_STOP_DET;
        break;

    case I2C_MASK_START_DET:
        Ins->IC_CLR_START_DET;
        break;

    case I2C_MASK_GEN_CALL:
        Ins->IC_CLR_GEN_CALL;
        break;
    }
}

/*******************************************************************************
    @ Module               :  Hal_IIC_Write_TXFIFO
    @ Description    :  [para in]:Ins,the instance wanted to write,
*******************************************************************************/
void Hal_IIC_Write_TXFIFO(AP_I2C_TypeDef* Ins,uint8_t data)
{
    Ins->IC_DATA_CMD = ((I2C_Data_WRITE << 8) | data);
}

/*******************************************************************************
    @ Module               :  hal_I2C0_IRQHandler
    @ Description    :  This function process for i2c0 interrupt
*******************************************************************************/
void __attribute__((used)) Hal_I2C0_IRQHandler(void)
{
    I2C_Evt_t irq_s;
    irq_s.type= (I2C_EVT)(AP_I2C0->IC_INTR_STAT);
    (*I2C0_IRQ_Handle)(&irq_s);
}

/*******************************************************************************
    @ Module               :  hal_I2C1_IRQHandler
    @ Description    :  This function process for i2c1 interrupt
*******************************************************************************/
void __attribute__((used)) Hal_I2C1_IRQHandler(void)
{
    I2C_Evt_t irq_s;
    irq_s.type= (I2C_EVT)(AP_I2C1->IC_INTR_STAT);
    (*I2C1_IRQ_Handle)(&irq_s);
}

/*************
 i2c_io.c
 SDK_LICENSE
***************/

//#include "iic.h"
#include "bus_dev.h"
#include "gpio.h"
#include "error.h"

//static gpio_pin_e s_pin_sda = GPIO_DUMMY;
//static gpio_pin_e s_pin_scl = GPIO_DUMMY;
//static bool s_dir_scl;
#define IIC_SDA_PIN GPIO_DUMMY
#define IIC_SCL_PIN GPIO_DUMMY

#define TWI_SDA_LOW()    hal_gpio_fast_write(IIC_SDA_PIN,0)
#define TWI_SDA_HIGH()   hal_gpio_fast_write(IIC_SDA_PIN,1)

#define TWI_SCL_LOW()     hal_gpio_write(IIC_SCL_PIN,0)
#define TWI_SCL_HIGH()    hal_gpio_write(IIC_SCL_PIN,1)

#define TWI_SCL_READ()    hal_gpio_read(IIC_SCL_PIN)
#define TWI_SDA_READ()    hal_gpio_read(IIC_SDA_PIN)

#define TWI_SDA_OUTPUT()  hal_gpio_pin_init(IIC_SDA_PIN,OEN)
#define TWI_SDA_INPUT()   hal_gpio_pin_init(IIC_SDA_PIN,IE); hal_gpio_pull_set(IIC_SDA_PIN, STRONG_PULL_UP)

#define TWI_SCL_OUTPUT()  hal_gpio_pin_init(IIC_SCL_PIN,OEN)
#define TWI_SCL_INPUT()   hal_gpio_pin_init(IIC_SCL_PIN,IE); hal_gpio_pull_set(IIC_SCL_PIN, STRONG_PULL_UP)
/*
    #define SCL_H   GPIOC->BSRR = GPIO_Pin_12
    #define SCL_L   GPIOC->BRR = GPIO_Pin_12

    #define SDA_H   GPIOC->BSRR = GPIO_Pin_11
    #define SDA_L   GPIOC->BRR = GPIO_Pin_11

    #define SCL_read      GPIOC->IDR  & GPIO_Pin_12
    #define SDA_read      GPIOC->IDR  & GPIO_Pin_11
*/
#define SCL_H    TWI_SCL_HIGH()
#define SCL_L    TWI_SCL_LOW()

#define SDA_H   TWI_SCL_HIGH()
#define SDA_L   TWI_SCL_LOW()

//#define SCL_read      GPIOC->IDR  & GPIO_Pin_12
#define SDA_read      TWI_SDA_READ()
//GPIOC->IDR  & GPIO_Pin_11

#if 0
static void scl_w(int value)
{
}

static int scl_r(void)
{
}

static void sda_w(int value)
{
}

static int sda_r(void)
{
}
#endif

void I2C_delay(void)
{
    uint8_t i=5;

    while(i)
    {
        i--;
    }
}

void I2C_delay_100us(uint32_t nCount)
{
    volatile int i = 450;

    while(nCount)
    {
        nCount--;

        for(; i; i--);
    }
}

bool i2c_start(void)
{
    SDA_H;
    SCL_H;
    I2C_delay();

    if(!SDA_read)
    {
        return false;
    }

    SDA_L;
    I2C_delay();

    if(SDA_read)
    {
        return false;
    }

    SDA_L;
    I2C_delay();
    return true;
}

void i2c_stop(void)
{
    SCL_L;
    I2C_delay();
    SDA_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SDA_H;
    I2C_delay();
}

void i2c_ack(void)
{
    SCL_L;
    I2C_delay();
    SDA_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;
    I2C_delay();
}

void i2c_nack(void)
{
    SCL_L;
    I2C_delay();
    SDA_H;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;
    I2C_delay();
}

bool i2c_wait_ack(void)
{
    SCL_L;
    I2C_delay();
    SDA_H;
    I2C_delay();
    SCL_H;
    I2C_delay();

    if(SDA_read)
    {
        SCL_L;
        return false;
    }

    SCL_L;
    return true;
}

void i2c_tx(uint8_t SendByte)
{
    uint8_t i=8;

    while(i--)
    {
        SCL_L;
        I2C_delay();

        if(SendByte&0x80)
            SDA_H;
        else
            SDA_L;

        SendByte<<=1;
        I2C_delay();
        SCL_H;
        I2C_delay();
    }

    SCL_L;
}

uint8_t i2c_rx(void)
{
    uint8_t i=8;
    uint8_t ReceiveByte=0;
    SDA_H;

    while(i--)
    {
        ReceiveByte<<=1;
        SCL_L;
        I2C_delay();
        SCL_H;
        I2C_delay();

        if(SDA_read)
        {
            ReceiveByte|=0x01;
        }
    }

    SCL_L;
    return ReceiveByte;
}

int i2c_io_pin_init(gpio_pin_e pin_sda, gpio_pin_e pin_clk)
{
    return PPlus_SUCCESS;
}
int i2c_io_pin_deinit(gpio_pin_e pin_sda, gpio_pin_e pin_clk)
{
    return PPlus_SUCCESS;
}

bool i2c_io_read(uint8_t slave_addr, uint8_t reg, uint8_t* data, uint8_t size)
{
    if(!i2c_start())
    {
        return false;
    }

    i2c_tx(slave_addr);

    if(!i2c_wait_ack())
    {
        i2c_stop();
        return false;
    }

    i2c_tx(reg);
    i2c_wait_ack();
    i2c_start();
    i2c_tx(slave_addr|0x01);
    i2c_wait_ack();

    while(size)
    {
        *data = i2c_rx();

        if(size == 1)
        {
            i2c_nack();
        }
        else
        {
            i2c_ack();
        }

        data++;
        size--;
    }

    i2c_stop();
    return true;
}

bool i2c_io_write(uint8_t slave_addr, uint8_t reg, uint8_t value)
{
    if(!i2c_start())return false;

    i2c_tx(slave_addr);

    if(!i2c_wait_ack())
    {
        i2c_stop();
        return false;
    }

    i2c_tx(reg);//i2c_tx((uint8_t)(reg & 0x00FF));
    i2c_wait_ack();
    i2c_tx(value);
    i2c_wait_ack();
    i2c_stop();
    return true;
}




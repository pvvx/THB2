
#ifndef __PHY62XX_H
#define __PHY62XX_H

#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup Configuration_section_for_CMSIS
    @{
*/
/**
    @brief Configuration of the Cortex-M0+ Processor and Core Peripherals
*/
#define __CM0_REV                 0 /*!< Core Revision r0p0                            */
#define __MPU_PRESENT             0 /*!< M0  provides an MPU                    */
#define __VTOR_PRESENT            0 /*!< Vector  Table  Register supported             */
#define __NVIC_PRIO_BITS          2 /*!< M0 uses 2 Bits for the Priority Levels */
#define __Vendor_SysTickConfig    0 /*!< Set to 1 if different SysTick Config is used  */


/**
    @}
*/

/** @addtogroup Peripheral_interrupt_number_definition
    @{
*/

/**
    @brief STM32L0xx Interrupt Number Definition, according to the selected device
          in @ref Library_configuration_section
*/

/*!< Interrupt Number Definition */
typedef enum
{
    /******  Cortex-M0 Processor Exceptions Numbers ******************************************************/
    NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                */
    HardFault_IRQn              = -13,    /*!< 3 Cortex-M0+ Hard Fault Interrupt                        */
    SVC_IRQn                    = -5,     /*!< 11 Cortex-M0+ SV Call Interrupt                          */
    PendSV_IRQn                 = -2,     /*!< 14 Cortex-M0+ Pend SV Interrupt                          */
    SysTick_IRQn                = -1,     /*!< 15 Cortex-M0+ System Tick Interrupt                      */

    /******  STM32L-0 specific Interrupt Numbers *********************************************************/
    WWDG_IRQn                   = 0,      /*!< Window WatchDog Interrupt                                     */
    PVD_IRQn                    = 1,      /*!< PVD through EXTI Line detect Interrupt                        */
    RTC_IRQn                    = 2,      /*!< RTC through EXTI Line Interrupt                               */
    FLASH_IRQn                  = 3,      /*!< FLASH Interrupt                                               */
    RCC_IRQn                    = 4,      /*!< RCC Interrupt                                                 */
    EXTI0_1_IRQn                = 5,      /*!< EXTI Line 0 and 1 Interrupts                                  */
    EXTI2_3_IRQn                = 6,      /*!< EXTI Line 2 and 3 Interrupts                                  */
    EXTI4_15_IRQn               = 7,      /*!< EXTI Line 4 to 15 Interrupts                                  */
    DMA1_Channel1_IRQn          = 9,      /*!< DMA1 Channel 1 Interrupt                                      */
    DMA1_Channel2_3_IRQn        = 10,     /*!< DMA1 Channel 2 and Channel 3 Interrupts                       */
    DMA1_Channel4_5_6_7_IRQn    = 11,     /*!< DMA1 Channel 4, Channel 5, Channel 6 and Channel 7 Interrupts */
    ADC1_COMP_IRQn              = 12,     /*!< ADC1, COMP1 and COMP2 Interrupts                              */
    LPTIM1_IRQn                 = 13,     /*!< LPTIM1 Interrupt                                              */
    TIM2_IRQn                   = 15,     /*!< TIM2 Interrupt                                                */
    TIM6_IRQn                   = 17,     /*!< TIM6  Interrupt                                               */
    TIM21_IRQn                  = 20,     /*!< TIM21 Interrupt                                               */
    TIM22_IRQn                  = 22,     /*!< TIM22 Interrupt                                               */
    I2C1_IRQn                   = 23,     /*!< I2C1 Interrupt                                                */
    I2C2_IRQn                   = 24,     /*!< I2C2 Interrupt                                                */
    SPI1_IRQn                   = 25,     /*!< SPI1 Interrupt                                                */
    SPI2_IRQn                   = 26,     /*!< SPI2 Interrupt                                                */
    USART1_IRQn                 = 27,     /*!< USART1 Interrupt                                              */
    USART2_IRQn                 = 28,     /*!< USART2 Interrupt                                              */
    LPUART1_IRQn                = 29,     /*!< LPUART1 Interrupts                                            */
} IRQn_Type;

/**
    @}
*/

#include "core_cm0.h"
//#include "system_m0.h"
#include <stdint.h>



/**
    @}
*/

/******************************************************************************/
/*  For a painless codes migration between the STM32L0xx device product       */
/*  lines, the aliases defined below are put in place to overcome the         */
/*  differences in the interrupt handlers and IRQn definitions.               */
/*  No need to update developed interrupt code when moving across             */
/*  product lines within the same STM32L0 Family                              */
/******************************************************************************/

/* Aliases for __IRQn */

/**
    @}
*/

/**
    @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PHY62XX_H */



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/




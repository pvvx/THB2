/**************************************************************************/ /**
 * @file     startup_ARMCM0.c
 * @brief    CMSIS Core Device Startup File for
 *           ARMCM0 Device
 * @version  V1.1.0
 * @date     23. January 2019
 ******************************************************************************/
/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
*/

#include "ARMCM0.h"

/*----------------------------------------------------------------------------
  Linker generated Symbols
 *----------------------------------------------------------------------------*/
extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __StackTop;

#define __STARTUP_CLEAR_BSS

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void (*pFunc)(void);
extern const uint32_t *const jump_table_base[256];
extern uint32_t global_config[256];

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern void _start(void) __attribute__((noreturn)); /* PreeMain (C library entry point) */
// extern const uint32_t *const jump_table_base[256];
// extern uint32_t global_config[256];

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Default_Handler(void) __attribute__((noreturn));
void Reset_Handler(void) __attribute__((noreturn));

/*----------------------------------------------------------------------------
  User Initial Stack & Heap
 *----------------------------------------------------------------------------*/
//<h> Stack Configuration
//  <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
//</h>
#define __STACK_SIZE 0x00000400
static uint8_t stack[__STACK_SIZE] __attribute__((aligned(8), used, section(".stack")));

//<h> Heap Configuration
//  <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
//</h>
#define __HEAP_SIZE 0x00000C00
#if __HEAP_SIZE > 0
static uint8_t heap[__HEAP_SIZE] __attribute__((aligned(8), used, section(".heap")));
#endif

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

void WDT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void MCIA_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void MCIB_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void AACI_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CLCD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ENET_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USBDC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USBHC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CHLCD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FLEXRAY_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LIN_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CPU_CLCD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
extern const pFunc __Vectors[48];
const pFunc __Vectors[48] __attribute__((used, section(".vectors"))) = {
    (pFunc)(&__StackTop), /*     Initial Stack Pointer */
    Reset_Handler,        /*     Reset Handler */
    NMI_Handler,          /* -14 NMI Handler */
    HardFault_Handler,    /* -13 Hard Fault Handler */
    0,                    /*     Reserved */
    0,                    /*     Reserved */
    0,                    /*     Reserved */
    0,                    /*     Reserved */
    0,                    /*     Reserved */
    0,                    /*     Reserved */
    0,                    /*     Reserved */
    SVC_Handler,          /*  -5 SVCall Handler */
    0,                    /*     Reserved */
    0,                    /*     Reserved */
    PendSV_Handler,       /*  -2 PendSV Handler */
    SysTick_Handler,      /*  -1 SysTick Handler */

    /* Interrupts */
    WDT_IRQHandler,   /*   0 Interrupt 0 */
    RTC_IRQHandler,   /*   1 Interrupt 1 */
    TIM0_IRQHandler,  /*   2 Interrupt 2 */
    TIM2_IRQHandler,  /*   3 Interrupt 3 */
    MCIA_IRQHandler,  /*   4 Interrupt 4 */
    MCIB_IRQHandler,  /*   5 Interrupt 5 */
    UART0_IRQHandler, /*   6 Interrupt 6 */
    UART1_IRQHandler, /*   7 Interrupt 7 */
    UART2_IRQHandler, /*   8 Interrupt 8 */
    UART3_IRQHandler, /*   9 Interrupt 9 */
    UART4_IRQHandler, /*   10 Interrupt 10 */
    AACI_IRQHandler,  /*   10 Interrupt 10 */
    CLCD_IRQHandler,
    ENET_IRQHandler,
    USBDC_IRQHandler,
    USBHC_IRQHandler,
    CHLCD_IRQHandler,
    FLEXRAY_IRQHandler,
    CAN_IRQHandler,
    LIN_IRQHandler,
    I2C_IRQHandler,
    CPU_CLCD_IRQHandler,
    SPI_IRQHandler,
};

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void)
{
    uint32_t *pSrc, *pDest;
    uint32_t *pTable __attribute__((unused));

    SystemInit(); /* CMSIS System Initialization */

/* Firstly it copies data from read only memory to RAM.
 * There are two schemes to copy. One can copy more than one sections.
 * Another can copy only one section. The former scheme needs more
 * instructions and read-only data to implement than the latter.
 * Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.
 */

#ifdef __STARTUP_COPY_MULTIPLE
    /* Multiple sections scheme.
 *
 * Between symbol address __copy_table_start__ and __copy_table_end__,
 * there are array of triplets, each of which specify:
 *   offset 0: LMA of start of a section to copy from
 *   offset 4: VMA of start of a section to copy to
 *   offset 8: size of the section to copy. Must be multiply of 4
 *
 * All addresses must be aligned to 4 bytes boundary.
 */
    pTable = &__copy_table_start__;

    for (; pTable < &__copy_table_end__; pTable = pTable + 3)
    {
        pSrc = (uint32_t *)*(pTable + 0);
        pDest = (uint32_t *)*(pTable + 1);
        for (; pDest < (uint32_t *)(*(pTable + 1) + *(pTable + 2));)
        {
            *pDest++ = *pSrc++;
        }
    }
#else
    /* Single section scheme.
 *
 * The ranges of copy from/to are specified by following symbols
 *   __etext: LMA of start of the section to copy from. Usually end of text
 *   __data_start__: VMA of start of the section to copy to
 *   __data_end__: VMA of end of the section to copy to
 *
 * All addresses must be aligned to 4 bytes boundary.
 */
    pSrc = &__etext;
    pDest = &__data_start__;

    for (; pDest < &__data_end__;)
    {
        *pDest++ = *pSrc++;
    }
#endif /*__STARTUP_COPY_MULTIPLE */

/* This part of work usually is done in C library startup code.
 * Otherwise, define this macro to enable it in this startup.
 *
 * There are two schemes too.
 * One can clear multiple BSS sections. Another can only clear one section.
 * The former is more size expensive than the latter.
 *
 * Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 * Otherwise define macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
    /* Multiple sections scheme.
 *
 * Between symbol address __copy_table_start__ and __copy_table_end__,
 * there are array of tuples specifying:
 *   offset 0: Start of a BSS section
 *   offset 4: Size of this BSS section. Must be multiply of 4
 */
    pTable = &__zero_table_start__;

    for (; pTable < &__zero_table_end__; pTable = pTable + 2)
    {
        pDest = (uint32_t *)*(pTable + 0);
        for (; pDest < (uint32_t *)(*(pTable + 0) + *(pTable + 1));)
        {
            *pDest++ = 0;
        }
    }
#elif defined(__STARTUP_CLEAR_BSS)
    /* Single BSS section scheme.
 *
 * The BSS section is specified by following symbols
 *   __bss_start__: start of the BSS section.
 *   __bss_end__: end of the BSS section.
 *
 * Both addresses must be aligned to 4 bytes boundary.
 */
    pDest = &__bss_start__;

    for (; pDest < &__bss_end__;)
    {
        *pDest++ = 0UL;
    }
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */

    /* Tested OK */
    // while (1)
    // {
    //     hal_gpio_toggle(GPIO_P14);
    // }

    // Avoid optimizating out the jump_table and global_config
    // The following lines crash
    // volatile uint32_t *t1 = jump_table_base[0];
    // volatile uint32_t t2 = global_config[0];

    _start(); /* Enter PreeMain (C library entry point) */
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
//    BM_SET(REG_FMUX_EN_FUC(GPIO_P14), Bit_DISABLE); //set bit
//    BM_SET(reg_gpio_ioe_porta, OEN);
//    while (1)
//    {
//        //Toggle GPIO 2/3 PWM
//        BM_SET(reg_gpio_swporta_dr, GPIO_P14);
//        BM_SET(reg_gpio_swporta_dr, GPIO_P14);
//        BM_CLR(reg_gpio_swporta_dr, GPIO_P14);
//    }
}

/*----------------------------------------------------------------------------
  _exit implementation for libc
 *----------------------------------------------------------------------------*/
void _exit(int status)
{
    (void)status;
    while (1)
        ;
}

// ROM lib expect __initial_sp symbol to be defined
// Either add the following line
// const pFunc __initial_sp = (pFunc)(&__StackTop);
// or provide __initial_sp in the linker

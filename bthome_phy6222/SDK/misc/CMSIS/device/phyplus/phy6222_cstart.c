
/****************************************************************************
    Included Files
 ****************************************************************************/

#if 1
#include "rom_sym_def.h"
#include <stddef.h>
#include "phy6222_cstart.h"
#include "clock.h"
#include "log.h"
#include "flash.h"
#include "jump_function.h"
#include "global_config.h"

/****************************************************************************
    Pre-processor Definitions
 ****************************************************************************/

//extern const uint32_t _sramscttexts;
//extern const uint32_t _sramscttext;
//extern const uint32_t _eramscttext;

//extern const uint32_t _sjtblss;
//extern const uint32_t _sjtbls;
//extern const uint32_t _ejtbls;
extern int main(void);
extern const uint32_t _sbss;
extern const uint32_t _ebss;

//extern const uint32_t _eronly;
//extern const uint32_t _sdata;
//extern const uint32_t _edata;

/****************************************************************************
    Name: c_start

    Description:
     This is the reset entry point.

 ****************************************************************************/

//extern void *osal_memset(void *s, uint8 c, size_t n);
//extern void* osal_memcpy(void* dest, const void* src, size_t n);
//extern uint32 global_config[SOFT_PARAMETER_NUM];

void c_start(void)
{
    uint8_t* dest;
    uint8_t* edest;
///    spif_config(SYS_CLK_DLL_64M, 1, XFRD_FCMD_READ_DUAL, 0, 0);
///    AP_PCR->CACHE_BYPASS = 1; //just bypass cache
    /*  Clear .bss.  We'll do this inline (vs. calling memset) just to be
        certain that there are no issues with the state of global variables.
    */
    dest = (uint8_t*)&_sbss;
    edest = (uint8_t*)&_ebss;
    osal_memset(dest, 0, edest - dest);
    dest = (uint8_t*)0x1fff0400;
    osal_memset(dest, 0, SOFT_PARAMETER_NUM * 4);
    /*  Move the initialized data section from his temporary holding spot in
        FLASH into the correct place in SRAM.  The correct place in SRAM is
        give by _sdata and _edata.  The temporary location is in FLASH at the
        end of all of the other read-only data (.text, .rodata) at _eronly.
    */
#if 0
    const uint8_t* src = (const uint8_t*)&_eronly;
    dest = (uint8_t*)&_sdata;
    edest = (uint8_t*)&_edata;
    osal_memcpy(dest, src, edest - dest);
#endif
    main();

    /* Shouldn't get here */

    for (; ; );
}

#else
void c_start(void)
{
    main();

    /* Shouldn't get here */

    for (; ; );
}

#endif

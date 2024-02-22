
/****************************************************************************
    Included Files
 ****************************************************************************/
#include <stdint.h>
#include <string.h>

/****************************************************************************
    Pre-processor Definitions
 ****************************************************************************/

extern int main(void);
extern const uint32_t _sbss;
extern const uint32_t _ebss;

/****************************************************************************
    Name: c_start

    Description:
     This is the reset entry point.

 ****************************************************************************/

#ifdef __GNUC__
void c_start(void) __attribute__ ((naked));
#endif
void c_start(void)
{
    /*  Clear .bss.  We'll do this inline (vs. calling memset) just to be
        certain that there are no issues with the state of global variables.
    */
    uint8_t* dest = (uint8_t*)&_sbss;
    uint8_t* edest = (uint8_t*)&_ebss;

    memset(dest, 0, edest - dest);

    main();

    /* Shouldn't get here */

    while(1);
}


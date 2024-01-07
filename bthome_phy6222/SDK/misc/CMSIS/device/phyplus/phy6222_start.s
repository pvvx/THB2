
/****************************************************************************
 * Included Files
 ****************************************************************************/


/****************************************************************************
 * Public Symbols
 ****************************************************************************/

	.file		"phy6222_start.s"

	/* *.ld: g_top_irqstack = ORIGIN(sram) + LENGTH(sram) */
    .global g_top_irqstack

.text
	.align	2
	.code	16
	.globl		__start
	.thumb_func
	.type	__start, %function
__start:

	ldr     r1, = g_top_irqstack
	msr		msp, r1					/* r2>>sp */
	bl		c_start				/* R0=IRQ, R1=register save area on stack */


	.size	__start, .-__start


.section .isr_vector
    .align  4
    .globl  __Vectors
    .type   __Vectors, %object
__Vectors:
    .long   0
    .long   __start

   .size   __Vectors, . - __Vectors


.end



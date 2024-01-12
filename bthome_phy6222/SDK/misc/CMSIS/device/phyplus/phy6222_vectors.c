
#define locate_data(n) __attribute__ ((section(n)))
extern unsigned int g_irqstack_top;
extern void __start(void);
const unsigned _vectors[] locate_data(".isr_vector") =
{
    /* Initial stack */
    (unsigned)(&g_irqstack_top),

    /* Reset exception handler */
    (unsigned)(&__start),

};

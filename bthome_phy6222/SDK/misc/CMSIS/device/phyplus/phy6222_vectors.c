
#define locate_data(n) __attribute__ ((section(n)))
extern unsigned int g_top_irqstack;
extern void __start(void);
const unsigned _vectors[] locate_data(".isr_vector") =
{
    /* Initial stack */

    (unsigned)(&g_top_irqstack),

    /* Reset exception handler */

    (unsigned)& __start,

};

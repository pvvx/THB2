

MEMORY
{
  jumptbl (rwx) : ORIGIN = 0x1fff0000, LENGTH = 0x00400
  gcfgtbl (rw)  : ORIGIN = 0x1fff0400, LENGTH = 0x00400
  flash (rx)    : ORIGIN = 0x11020000, LENGTH = 0x20000
  sram (rwx)  	: ORIGIN = 0x1fff1838, LENGTH = 0x0E7C8
}

OUTPUT_ARCH(arm)
EXTERN(_vectors)
ENTRY(__start)

SECTIONS
{
    .jumptbl : {
       KEEP(*(jump_table_mem_area))
    } > jumptbl  


    .gcfgtbl : {
      *(global_config_area)
    } > gcfgtbl

    .textentry : {
        *(*.isr_vector)
    } > sram


    .init_section : {
        _sinit = ABSOLUTE(.);
        *(.init_array .init_array.*)
        _einit = ABSOLUTE(.);
    } > flash

    .ARM.extab : {
        *(.ARM.extab*)
    } > flash

    __exidx_start = ABSOLUTE(.);
    .ARM.exidx : {
        *(.ARM.exidx*)
    } > flash
    __exidx_end = ABSOLUTE(.);

 
    ._sjtblsstore : {
       _sjtblss = ABSOLUTE(.);
    } > flash   


    .data : {
        _sdata = ABSOLUTE(.);
        _stextram = ABSOLUTE(.);
        *phy6222_start.o(.text)
        *.o(_section_standby_code_)
        *.o(_section_sram_code_)

        *patch.o(.text.*)
        *patch_ext_adv.o(.text.*)

        *rf_phy_driver.o(.text.*)
        *pwrmgr.o(.text .text.*)
        *timer.o(.text .text.*)

        *flash.o(.text .text.*)
        *clock.o(.text.*)

        *phy_sec_ext.o(.text .text.*)

        *libc.a:lib_a-memset.o(.text.*)
        *libc.a:lib_a-memcpy-stub.o(.text.*)
        *libgcc.a:_udivsi3.o(.text)
        *libgcc.a:_divsi3.o(.text)
        *libgcc.a:_dvmd_tls.o(.text)
/*        *libgcc.a:_thumb1_case_sqi.o(.text)
        *libgcc.a:_thumb1_case_uqi.o(.text) */
   
        _etextram = ABSOLUTE(.);

        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        CONSTRUCTORS
        . = ALIGN(4);
        _edata = ABSOLUTE(.);
    } > sram 

    .bss : {
        _sbss = ABSOLUTE(.);
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = ABSOLUTE(.);
    } > sram
 
 /*   
    .int_stack : {
        . = ALIGN(4);
        *(int_stack)
        . = ALIGN(4);
        _stack_top = ABSOLUTE(.);
    } > sram
*/
    /* stack 32 kbytes */
    g_top_irqstack = ORIGIN(sram) + LENGTH(sram);

   .common_text : {
        _stext = ABSOLUTE(.);   
       *(.text .text.*)
       *(.rodata .rodata.*)
       *(.fixup)
        *(.gnu.warning)
        *(.rodata .rodata.*)
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.got)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        _etext = ABSOLUTE(.);
   } > flash   
 
    /* Stabs debugging sections. */
    .stab 0 : { *(.stab) }
    .stabstr 0 : { *(.stabstr) }
    .stab.excl 0 : { *(.stab.excl) }
    .stab.exclstr 0 : { *(.stab.exclstr) }
    .stab.index 0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment 0 : { *(.comment) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_info 0 : { *(.debug_info) }
    .debug_line 0 : { *(.debug_line) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_aranges 0 : { *(.debug_aranges) }
    
}


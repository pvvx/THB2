/* SRAM + XIP linker script */
/* https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/6n33n7fds/index.html */
PHDRS 
{
  romdata PT_LOAD FLAGS(6);
  text    PT_LOAD FLAGS(5);
  data    PT_LOAD FLAGS(6);
  xip     PT_LOAD FLAGS(5);
  rodata  PT_LOAD FLAGS(4);
}

MEMORY
{
  jumptbl (rw)  : ORIGIN = 0x1fff0000, LENGTH = 0x00400
  gcfgtbl (rw)  : ORIGIN = 0x1fff0400, LENGTH = 0x00400
  sram    (rwx) : ORIGIN = 0x1fff1838, LENGTH = 0x0E7C8
  flash   (rx)  : ORIGIN = 0x11003000, LENGTH = 0x0F000
  sram2   (rwx) : ORIGIN = 0x20000000, LENGTH = 0x10000
  sram3   (rwx) : ORIGIN = 0x20010000, LENGTH = 0x02000
  sram4   (rwx) : ORIGIN = 0x20012000, LENGTH = 0x00800
}

OUTPUT_ARCH(arm)
EXTERN(_vectors)
ENTRY(__start)

SECTIONS
{
    .jumptbl : {
        *(jump_table_mem_area)
    } > jumptbl : romdata

    .gcfgtbl (NOLOAD) : {
        *(global_config_area)
    } > gcfgtbl

    .textentry : {
        *(*.isr_vector)
    } > sram : text

    _sdata = ABSOLUTE(.);
    .text : {
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

        *libgcc.a:*.o(.text .text.*)
   
        _etextram = ABSOLUTE(.);
    } > sram : text

    .data : {
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        CONSTRUCTORS
    } > sram  : data
    . = ALIGN(4);
    _edata = ABSOLUTE(.);

    .bss : {
        _sbss = ABSOLUTE(.);
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = ABSOLUTE(.);
    } > sram

    .irq_stack : {
        *(g_irqstack_base)
    } > sram
	   
    g_stack = ORIGIN(sram) + LENGTH(sram);

    .xip : {
        _stext = ABSOLUTE(.);   

        *.o(_func_xip_code_)
        *.o(_section_xip_code_)
        *(.text .text.*)

        _etext = ABSOLUTE(.);
    } > flash : xip

    .rodata : {
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
    } > flash : rodata

    .init_section : {
        _sinit = ABSOLUTE(.);
        *(.init_array .init_array.*)
        _einit = ABSOLUTE(.);
    } > flash : rodata

    .ARM.extab : {
        *(.ARM.extab*)
    } > flash : rodata

    .ARM.exidx : {
        __exidx_start = ABSOLUTE(.);
        *(.ARM.exidx*)
        __exidx_end = ABSOLUTE(.);
    } > flash : rodata
 
    ._sjtblsstore : {
       _sjtblss = ABSOLUTE(.);
    } > flash : rodata
 
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

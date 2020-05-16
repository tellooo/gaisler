/* Script for -z combreloc: combine and sort reloc sections */
OUTPUT_FORMAT("elf32-sparc")
SEARCH_DIR("/opt/sparc-elf-4.4.2//sparc-elf/lib");
ENTRY(start)
MEMORY
{
  rom     : ORIGIN = 0x00000000, LENGTH = 256M
  ram     : ORIGIN = 0x40000000, LENGTH = 2048M
}
SECTIONS
{
  .text   : {
    CREATE_OBJECT_SYMBOLS
    *(.text .text.*)
     etext  =  .;
    . = ALIGN (16);
    *(.eh_frame)
    . = ALIGN (16);
    *(.gnu.linkonce.t*)
     /* C++ constructors */
     ___CTOR_LIST__ = .;
     LONG((___CTOR_END__ - ___CTOR_LIST__) / 4 - 2)
     KEEP(*(SORT(.ctors.*)))
     KEEP(*(.ctors))
     LONG(0)
     ___CTOR_END__ = .;
     ___DTOR_LIST__ = .;
     LONG((___DTOR_END__ - ___DTOR_LIST__) / 4 - 2)
     KEEP(*(SORT(.dtors.*)))
     KEEP(*(.dtors))
     LONG(0)
     ___DTOR_END__ = .;
     /* linux style initcalls constructors */
     __leonbare_initcall_start = .;
         KEEP(*(.initcall1.init))
    	KEEP(*(.initcall2.init))
    	KEEP(*(.initcall3.init))
    	KEEP(*(.initcall4.init))
    	KEEP(*(.initcall5.init))
    	KEEP(*(.initcall6.init))
    	KEEP(*(.initcall7.init))
     __leonbare_initcall_end = .;
     _rodata_start  =  .;
    *(.rodata*)
    *(.gnu.linkonce.r*)
     _erodata = ALIGN( 0x10 ) ;
    *(.gnu.linkonce*)
    KEEP(*(.init))
    KEEP(*(.fini))
    *(.lit)
    *(.shdata)
    . = ALIGN (16);
     etext  =  .;
  }  > ram
  .gcc_except_table   : {
    *(.gcc_except_table)
  }  > ram
  .data   : {
    *(.data .data.* )
    edata  =  .;
    _edata  =  .;
    __edata  =  .;
  }  > ram
  .bss   :
  {
    . = ALIGN(0x8);
    bss_start = .;
    _bss_start = .;
    __bss_start = .;
    *(.bss .bss.* )
    *(COMMON)
  }  > ram
  . = ALIGN(0x8);
  end = .;
  _end = .;
  __end = .;
  __heap1 = .;
  .jcr . (NOLOAD) : { *(.jcr) }
  .stab 0 (NOLOAD) :
  {
    [ .stab ]
  }
  .stabstr 0 (NOLOAD) :
  {
    [ .stabstr ]
  }
  .debug           0 : { *(.debug) }
  .line            0 : { *(.line) }
  .debug_srcinfo   0 : { *(.debug_srcinfo) }
  .debug_sfnames   0 : { *(.debug_sfnames) }
  .debug_aranges   0 : { *(.debug_aranges) }
  .debug_pubnames  0 : { *(.debug_pubnames) }
  .debug_info      0 : { *(.debug_info) }
  .debug_abbrev    0 : { *(.debug_abbrev) }
  .debug_line      0 : { *(.debug_line) }
  .debug_frame     0 : { *(.debug_frame) }
  .debug_str       0 : { *(.debug_str) }
  .debug_loc       0 : { *(.debug_loc) }
  .debug_macinfo   0 : { *(.debug_macinfo) }
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
}

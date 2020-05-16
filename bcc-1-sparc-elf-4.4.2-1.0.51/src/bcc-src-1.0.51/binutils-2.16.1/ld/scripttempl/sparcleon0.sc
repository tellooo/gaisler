# Linker script for sparc leon.
test -z "$ENTRY" && ENTRY=start
cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}")
${LIB_SEARCH_DIRS}

ENTRY(${ENTRY})

/* Base address of the on-CPU peripherals */
LEON_REG  = 0x80000000;
_LEON_REG = 0x80000000;

MEMORY
{
  ram     : ORIGIN = 0x00000000, LENGTH = 3062M
}

SECTIONS
{
  .text ${RELOCATING-0} : {
    ${RELOCATING+CREATE_OBJECT_SYMBOLS}
    *(.text .text.*)
    ${RELOCATING+ etext  =  .;}
    . = ALIGN (16);
    *(.eh_frame)
    . = ALIGN (16);
    *(.gnu.linkonce.t*)

    ${CONSTRUCTING+ /* C++ constructors */}
    ${CONSTRUCTING+ ___CTOR_LIST__ = .;}
    ${CONSTRUCTING+ LONG((___CTOR_END__ - ___CTOR_LIST__) / 4 - 2)}
    ${CONSTRUCTING+ *(SORT(.ctors.*))}
    ${CONSTRUCTING+ *(.ctors)}
    ${CONSTRUCTING+ LONG(0)}
    ${CONSTRUCTING+ ___CTOR_END__ = .;}
    ${CONSTRUCTING+ ___DTOR_LIST__ = .;}
    ${CONSTRUCTING+ LONG((___DTOR_END__ - ___DTOR_LIST__) / 4 - 2)}
    ${CONSTRUCTING+ *(SORT(.dtors.*))}
    ${CONSTRUCTING+ *(.dtors)}
    ${CONSTRUCTING+ LONG(0)}
    ${CONSTRUCTING+ ___DTOR_END__ = .;}

    ${CONSTRUCTING+ /* linux style initcalls constructors */}
    ${CONSTRUCTING+ __leonbare_initcall_start = .;  }
    ${CONSTRUCTING+     *(.initcall1.init) }
    ${CONSTRUCTING+	*(.initcall2.init) }
    ${CONSTRUCTING+	*(.initcall3.init) }
    ${CONSTRUCTING+	*(.initcall4.init) }
    ${CONSTRUCTING+	*(.initcall5.init) }
    ${CONSTRUCTING+	*(.initcall6.init) }
    ${CONSTRUCTING+	*(.initcall7.init) }
    ${CONSTRUCTING+ __leonbare_initcall_end = .;    }
    
    ${RELOCATING+ _rodata_start  =  .;}
    *(.rodata*)
    *(.gnu.linkonce.r*)
    ${RELOCATING+ _erodata = ALIGN( 0x10 ) ;}
    
    *(.gnu.linkonce*)
    *(.init)
    *(.fini)
    *(.lit)
    *(.shdata)
    . = ALIGN (16);
    
    ${RELOCATING+ etext  =  .};
  } ${RELOCATING+ > ram }
  .gcc_except_table ${RELOCATING-0} : {
    *(.gcc_except_table)
  } ${RELOCATING+ > ram }
  .data ${RELOCATING-0} : {
    *(.data${RELOCATING+ .data.* })
    ${RELOCATING+edata  =  .;}
    ${RELOCATING+_edata  =  .;}
    ${RELOCATING+__edata  =  .;}
  } ${RELOCATING+ > ram }
  .bss ${RELOCATING-0} :
  { 					
    ${RELOCATING+. = ALIGN(0x8);}
    ${RELOCATING+bss_start = .;}
    ${RELOCATING+_bss_start = .;}
    ${RELOCATING+__bss_start = .;}
    *(.bss${RELOCATING+ .bss.* })
    *(COMMON)
  } ${RELOCATING+ > ram }
  ${RELOCATING+. = ALIGN(0x8);}
  ${RELOCATING+end = .;}
  ${RELOCATING+_end = .;}
  ${RELOCATING+__end = .;}
  ${RELOCATING+__heap1 = .;}
      
  .jcr . ${RELOCATING+(NOLOAD)} : { *(.jcr) }
  .stab 0 ${RELOCATING+(NOLOAD)} : 
  {
    [ .stab ]
  }
  .stabstr 0 ${RELOCATING+(NOLOAD)} :
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
EOF

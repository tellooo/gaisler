# Linker script for sparc leon nucleus.
test -z "$ENTRY" && ENTRY=kernel_start
cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}")
${LIB_SEARCH_DIRS}

ENTRY(${ENTRY})

/* Base address of the on-CPU peripherals */
LEON_REG  = 0x80000000;
_LEON_REG = 0x80000000;

MEMORY
{
  rom     : ORIGIN = 0x00000000, LENGTH = 256M
  ram     : ORIGIN = 0x40000000, LENGTH = 2048M
}

SECTIONS
{
    .text ${RELOCATING-0} :
    {
        *(.esal_vector)
        *(.esal_reset_exception)
        *(.esal_code)
        *(.text*)
        *(.gnu.linkonce.t*)
        KEEP (*(.init))
        __SDEINIT_LIST__ = .;
        KEEP (*(.fini))
        __SDEFINI_LIST__ = .;        

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
    
    } ${RELOCATING+ > ram }
    
    .rodata ${RELOCATING-0} :
    {
        ${RELOCATING+ _rodata_start  =  .;}
        *(.rodata*)
        *(.gnu.linkonce.r*)
        ${RELOCATING+ _erodata = ALIGN( 0x10 ) ;}
        *(.gnu.linkonce*)
    
        *(.eh_frame)
        *(.gcc_except_table)
        *(.jcr) 
        __CTOR_LIST__ = .;        
        KEEP (*crt0.o(.ctors))                                                           
        KEEP (*crtbegin*.o(.ctors)) 
        KEEP (*(EXCLUDE_FILE (*crtend*.o *crtn.o) .ctors)) 
        KEEP (*(SORT(.ctors.*))) 
        KEEP (*(.ctors)) 
        __DTOR_LIST__ = .;
        KEEP (*crt0.o(.dtors)) 
        KEEP (*crtbegin*.o(.dtors)) 
        KEEP (*(EXCLUDE_FILE (*crtend*.o *crtn.o) .dtors)) 
        KEEP (*(SORT(.dtors.*))) 
        KEEP (*(.dtors)) 
        _end = .;

    } ${RELOCATING+ > ram }

    .gcc_except_table ${RELOCATING-0} : {
        *(.gcc_except_table)
    } ${RELOCATING+ > ram }
  
    .data ${RELOCATING-0} :
    {
        _ld_rom_data_start = . ;
        _ld_ram_data_start = . ;
        *(.data*)
        *(.gnu.linkonce.d*)
        _gp = . + 0x8000;
        *(.sdata*)
        *(.gnu.linkonce.s.*)
        _ld_ram_data_end = . ;

    } ${RELOCATING+ > ram }

    .heap ${RELOCATING-0} (NOLOAD) :
    {
        _HEAP = .;
        _HEAP_START = .;
        . = . + 0x400;
        _HEAP_END = .;

    } ${RELOCATING+ > ram }

    .bss ${RELOCATING-0} (NOLOAD) :
    {
        _ld_bss_start = .;
        *(.sbss*)
        *(.gnu.linkonce.sb.*)
        *(.scommon*)
        *(.bss*)
        *(.gnu.linkonce.b.*)
        *(COMMON*)
	${RELOCATING+. = ALIGN(0x8);}
        _ld_bss_end = .;

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

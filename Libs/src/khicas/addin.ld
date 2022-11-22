OUTPUT_ARCH(sh3)
ENTRY(initialize)
MEMORY
{
        rom  : o = 0x00300200, l = 2048k
        ram  : o = 0x08100000, l = 16k  /* 32k-16k for stack */
}
SECTIONS
{
        .text : {
                *(.pretext)     /* init stuff */
                *(.text)
        } > rom
        .rodata : {
                *(.rodata)
                *(.rodata*)
                *(.rodata.str1.4)
		*(.eh_frame)
		*(C)
		_romdata = . ;  /* symbol for initialization data */
        } > rom
        .bss : {
                _bbss = . ;
                _bssdatasize = . ;
                LONG(0);        /* bssdatasize */
                *(.bss) *(COMMON);
                _ebss = . ;
        } > ram
        .data BLOCK(4) : AT(_romdata) {
                _bdata = . ;
                *(.data);
                _edata = . ;
        } > ram
}

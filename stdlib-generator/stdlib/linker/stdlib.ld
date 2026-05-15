SECTIONS
{
    /* Place .text at address 0 so that offsets = symbol values */
    . = 0;

    .text : {
        *(.text)
        *(.text.*)
    }

    /* Discard all other sections – we only need .text */
    /DISCARD/ : {
        *(.data)
        *(.bss)
        *(.rodata)
        *(.note*)
        *(.comment)
        *(.eh_frame)
        *(.debug*)
    }
}
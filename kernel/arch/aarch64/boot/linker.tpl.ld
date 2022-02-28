/*
 * This file will be firstly configured by CMake to fill in `init_objects`,
 * then preprocessed by C compiler to fill in some definitions from `image.h`.
 */

#include "image.h"

ENTRY(_start)

SECTIONS
{
    . = TEXT_OFFSET;
    img_start = .;
    init : {
        ${init_objects}
    }

    . = ALIGN(SZ_16K);

    init_end = ABSOLUTE(.);

    .text KERNEL_VADDR + init_end : AT(init_end) {
        *(.text*)
    }

    . = ALIGN(SZ_64K);
    .data : {
        *(.data*)
    }
    . = ALIGN(SZ_64K);

    .rodata : {
        *(.rodata*)
    }
    _edata = . - KERNEL_VADDR;

    _bss_start = . - KERNEL_VADDR;
    .bss : {
        *(.bss*)
    }
    _bss_end = . - KERNEL_VADDR;
    . = ALIGN(SZ_64K);

    img_end = . - KERNEL_VADDR;

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

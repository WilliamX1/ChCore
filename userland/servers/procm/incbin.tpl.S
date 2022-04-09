/* Binary include template. */

        .section .rodata
        .align 4
        .globl __binary_${binary_name}_start
__binary_${binary_name}_start:
        .incbin "${binary_path}"
__binary_${binary_name}_end:
        .globl __binary_${binary_name}_size
__binary_${binary_name}_size:
        .quad __binary_${binary_name}_end - __binary_${binary_name}_start

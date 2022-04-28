/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

/* Binary include template. */

        .section .rodata
        .align 4
        .globl __binary_ramdisk_cpio_start
__binary_ramdisk_cpio_start:
        .incbin "${binary_path}"
__binary_ramdisk_cpio_end:
        .globl __binary_ramdisk_cpio_size
__binary_ramdisk_cpio_size:
        .quad __binary_ramdisk_cpio_end - __binary_ramdisk_cpio_start

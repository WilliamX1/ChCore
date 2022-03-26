#pragma once

#define NR_SYSCALL 256

/* Character */
#define SYS_putc 0
#define SYS_getc 1

/* PMO */
/* - single */
#define SYS_create_pmo        10
#define SYS_create_device_pmo 11
#define SYS_map_pmo           12
#define SYS_unmap_pmo         13
#define SYS_write_pmo         14
#define SYS_read_pmo          15
/* - batch */
#define SYS_create_pmos 20
#define SYS_map_pmos    21
/* - address translation */
#define SYS_get_pmo_paddr 30
#define SYS_get_phys_addr 31

/* Capability */
#define SYS_cap_copy_to   60
#define SYS_cap_copy_from 61
#define SYS_transfer_caps 62

/* Multitask */
/* - create & exit */
#define SYS_create_cap_group 80
#define SYS_create_thread    82
#define SYS_thread_exit      83

/* POSIX */
/* - memory */
#define SYS_handle_brk    210
#define SYS_handle_mmap   211
#define SYS_handle_munmap 212

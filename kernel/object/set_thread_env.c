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

#include <common/util.h>

#include "thread_env.h"

/*
 * Setup the initial environment for a user process (main thread).
 *
 * According to Libc convention, we current set the environment
 * on the user stack.
 *
 */

#define AT_NULL     0 /* end of vector */
#define AT_IGNORE   1 /* entry should be ignored */
#define AT_EXECFD   2 /* file descriptor of program */
#define AT_PHDR     3 /* program headers for program */
#define AT_PHENT    4 /* size of program header entry */
#define AT_PHNUM    5 /* number of program headers */
#define AT_PAGESZ   6 /* system page size */
#define AT_BASE     7 /* base address of interpreter */
#define AT_FLAGS    8 /* flags */
#define AT_ENTRY    9 /* entry point of program */
#define AT_NOTELF   10 /* program is not ELF */
#define AT_UID      11 /* real uid */
#define AT_EUID     12 /* effective uid */
#define AT_GID      13 /* real gid */
#define AT_EGID     14 /* effective gid */
#define AT_PLATFORM 15 /* string identifying CPU for optimizations */
#define AT_HWCAP    16 /* arch dependent hints at CPU capabilities */
#define AT_CLKTCK   17 /* frequency at which times() increments */
/* AT_* values 18 through 22 are reserved */
#define AT_SECURE 23 /* secure mode boolean */
/* string identifying real platform, may differ from AT_PLATFORM. */
#define AT_BASE_PLATFORM 24
#define AT_RANDOM        25 /* address of 16 random bytes */
#define AT_HWCAP2        26 /* extension of AT_HWCAP */
#define AT_EXECFN        31 /* filename of program */

#if defined(CHCORE_ARCH_AARCH64)
const char PLAT[] = "aarch64";
#else
const char PLAT[] = "unknown";
#endif

/*
 * For setting up the stack (env) of some process.
 *
 * env: stack top address used by kernel
 * top_vaddr: stack top address mapped to user
 */
void prepare_env(char *env, u64 top_vaddr, struct process_metadata *meta,
                 char *name)
{
        int i;
        char *name_str;
        char *plat_str;
        u64 *p;

        /* clear env */
        memset(env, 0, ENV_SIZE);

        /* strings */
        /* the last 64 bytes */
        name_str = env + ENV_SIZE - 64;
        i = 0;
        while (name[i] != '\0') {
                name_str[i] = name[i];
                ++i;
        }

        /* the second last 64 bytes */
        plat_str = env + ENV_SIZE - 2 * 64;
        i = 0;
        while (PLAT[i] != '\0') {
                plat_str[i] = PLAT[i];
                ++i;
        }

        p = (u64 *)env;
        *p++ = (u64)ENV_MAGIC;

        /* pmo map addresses and caps */
        *p++ = (u64)ENV_NO_CAPS; /* No caps */

        /* argc */
        *p++ = 1;

        /* argv */
        *p++ = top_vaddr - 64;
        *p++ = (u64)NULL;

        /* envp */
        *p++ = (u64)NULL;

        /* auxv */
        *p++ = AT_SECURE;
        *p++ = 0;

        *p++ = AT_PAGESZ;
        *p++ = 0x1000;

        *p++ = AT_PHDR;
        *p++ = meta->phdr_addr;

        *p++ = AT_PHENT;
        *p++ = meta->phentsize;

        *p++ = AT_PHNUM;
        *p++ = meta->phnum;

        *p++ = AT_FLAGS;
        *p++ = meta->flags;

        *p++ = AT_ENTRY;
        *p++ = meta->entry;

        *p++ = AT_UID;
        *p++ = 1000;

        *p++ = AT_EUID;
        *p++ = 1000;

        *p++ = AT_GID;
        *p++ = 1000;

        *p++ = AT_EGID;
        *p++ = 1000;

        *p++ = AT_CLKTCK;
        *p++ = 100;

        *p++ = AT_HWCAP;
        *p++ = 0;

        *p++ = AT_PLATFORM;
        *p++ = top_vaddr - 64 * 2;

        *p++ = AT_RANDOM;
        *p++ = top_vaddr - 64; /* random 16 bytes */

        *p++ = AT_NULL;
        *p++ = 0;

        /* add more auxv here */
}

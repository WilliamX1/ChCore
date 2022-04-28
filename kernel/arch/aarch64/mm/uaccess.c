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
#include <common/vars.h>
#include <common/types.h>
#include <common/kprint.h>
#include <common/macro.h>

/*
 * Currently, we enable EL1 (kernel) to directly access EL0 (user)  memory.
 * But, El1 cannot execute EL0 code.
 */

int copy_from_user(char *kernel_buf, char *user_buf, size_t size)
{
        /* validate user_buf */
        BUG_ON((u64)user_buf >= KBASE);
        memcpy(kernel_buf, user_buf, size);
        return 0;
}

int copy_to_user(char *user_buf, char *kernel_buf, size_t size)
{
        /* validate user_buf */
        BUG_ON((u64)user_buf >= KBASE);
        memcpy(user_buf, kernel_buf, size);
        return 0;
}

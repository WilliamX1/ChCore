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

#include <arch/machine/esr.h>
#include <common/types.h>
#include <object/thread.h>
#include <mm/vmspace.h>

// declarations of fault handlers
int handle_trans_fault(struct vmspace *vmspace, vaddr_t fault_addr);

static inline vaddr_t get_fault_addr()
{
        vaddr_t addr;
        asm volatile("mrs %0, far_el1\n\t" : "=r"(addr));
        return addr;
}

// EC: Instruction Abort or Data Abort
void do_page_fault(u64 esr, u64 fault_ins_addr)
{
        vaddr_t fault_addr;
        int fsc; // fault status code

        fault_addr = get_fault_addr();
        fsc = GET_ESR_EL1_FSC(esr);
        switch (fsc) {
        case DFSC_TRANS_FAULT_L0:
        case DFSC_TRANS_FAULT_L1:
        case DFSC_TRANS_FAULT_L2:
        case DFSC_TRANS_FAULT_L3: {
                int ret;
                /* LAB 3 TODO BEGIN */

                ret = handle_trans_fault(current_thread->vmspace, fault_addr);
                
                /* LAB 3 TODO END */
                if (ret != 0) {
                        kinfo("do_page_fault: faulting ip is 0x%lx (real IP),"
                              "faulting address is 0x%lx,"
                              "fsc is trans_fault (0b%b)\n",
                              fault_ins_addr,
                              fault_addr,
                              fsc);
                        kprint_vmr(current_thread->vmspace);

                        kinfo("current_cap_group is %s\n",
                              current_cap_group->cap_group_name);

                        /*
                         * The backtrace function may help debugging.
                         * extern void backtrace(void);
                         * backtrace();
                         */

                        BUG_ON(ret != 0);
                }
                break;
        }
        case DFSC_PERM_FAULT_L1:
        case DFSC_PERM_FAULT_L2:
        case DFSC_PERM_FAULT_L3:
                kinfo("do_page_fault: faulting ip is 0x%lx (real IP),"
                      "faulting address is 0x%lx,"
                      "fsc is perm_fault (0b%b)\n",
                      fault_ins_addr,
                      fault_addr,
                      fsc);
                kprint_vmr(current_thread->vmspace);

                kinfo("current_cap_group is %s\n",
                      current_cap_group->cap_group_name);

                BUG_ON(1);
                break;
        case DFSC_ACCESS_FAULT_L1:
        case DFSC_ACCESS_FAULT_L2:
        case DFSC_ACCESS_FAULT_L3:
                kinfo("do_page_fault: fsc is access_fault (0b%b)\n", fsc);
                BUG_ON(1);
                break;
        default:
                kinfo("do_page_fault: faulting ip is 0x%lx (real IP),"
                      "faulting address is 0x%lx,"
                      "fsc is unsupported now (0b%b)\n",
                      fault_ins_addr,
                      fault_addr,
                      fsc);
                kprint_vmr(current_thread->vmspace);

                kinfo("current_cap_group is %s\n",
                      current_cap_group->cap_group_name);

                // backtrace();

                BUG_ON(1);
                break;
        }
}

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

#pragma once

/* ESR_EL1 Exception Syndrome Register */
/* Exception Class, ESR bits[31:26].
 * A subset of exception is recoreded here. */
#define EC_MASK                 (BIT(6) - 1)
#define ESR_EL1_EC_SHIFT        26
#define GET_ESR_EL1_EC(esr_el1) (((esr_el1) >> ESR_EL1_EC_SHIFT) & EC_MASK)

#define ESR_EL1_EC_UNKNOWN 0b000000
#define ESR_EL1_EC_WFI_WFE \
        0b000001 /* Trapped WFI or WFE instruction execution */
#define ESR_EL1_EC_ENFP                                             \
        0b000111 /* Access to SVE, Advanced SIMD, or floating-point \
                    functionality */
#define ESR_EL1_EC_ILLEGAL_EXEC 0b001110 /* Illegal Execution state */
#define ESR_EL1_EC_SVC_32 \
        0b010001 /* SVC instruction execution in AArch32 state */
#define ESR_EL1_EC_SVC_64 \
        0b010101 /* SVC instruction execution in AArch64 state */
#define ESR_EL1_EC_MRS_MSR_64                                           \
        0b011000 /* Trapped MSR, MRS or System instruction execution in \
                    AArch64 state */
#define ESR_EL1_EC_IABT_LEL \
        0b100000 /* Instruction Abort from a lower Exception level) */
#define ESR_EL1_EC_IABT_CEL \
        0b100001 /* Instruction Abort from a current Exception level */
#define ESR_EL1_EC_PC_ALIGN 0b100010 /* PC alignment fault exception */
#define ESR_EL1_EC_DABT_LEL \
        0b100100 /* Data Abort from a lower Exception level */
#define ESR_EL1_EC_DABT_CEL \
        0b100101 /* Data Abort from a current Exception level */
#define ESR_EL1_EC_SP_ALIGN 0b100110 /* SP alignment fault exception */
#define ESR_EL1_EC_FP_32                                                      \
        0b101000 /* Trapped floating-point exception taken from AArch32 state \
                  */
#define ESR_EL1_EC_FP_64                                                      \
        0b101100 /* Trapped floating-point exception taken from AArch64 state \
                  */
#define ESR_EL1_EC_SError 0b101111 /* SERROR */

/* Instruction Length for synchronous exceptions */
//#define ESR_EL1_IL                  BIT(25)

/* ESR_EL1_ISS, ESR bits[24:0] is relay on the specific cases */

/* IFSC & DFSC */
#define FSC_MASK                 (BIT(6) - 1)
#define ESR_EL1_FSC_SHIFT        0
#define GET_ESR_EL1_FSC(esr_el1) (((esr_el1) >> ESR_EL1_FSC_SHIFT) & FSC_MASK)

/* Instruction Abort */
/* Instruction Fault Status Code, IFSC, ESR bits[5:0] */
#define IFSC_TRANS_FAULT_L0 0b000100
#define IFSC_TRANS_FAULT_L1 0b000101
#define IFSC_TRANS_FAULT_L2 0b000110
#define IFSC_TRANS_FAULT_L3 0b000111

#define IFSC_ACCESS_FAULT_L1 0b001001
#define IFSC_ACCESS_FAULT_L2 0b001010
#define IFSC_ACCESS_FAULT_L3 0b001011

#define IFSC_PERM_FAULT_L1 0b001101
#define IFSC_PERM_FAULT_L2 0b001110
#define IFSC_PERM_FAULT_L3 0b001111

/* Data Abort */
/* WnR, ESR bit[6]. Write not Read. The cause of data abort. */
#define DABT_BY_READ  0b0
#define DABT_BY_WRITE 0b1

#define WnR_MASK                 1
#define ESR_EL1_WnR_SHIFT        6
#define GET_ESR_EL1_WnR(esr_el1) (((esr_el1) >> ESR_EL1_WnR_SHIFT) & WnR_MASK)

/* Data Fault Status Code, DFSC, ESR bits[5:0] */
#define DFSC_TRANS_FAULT_L0 0b000100
#define DFSC_TRANS_FAULT_L1 0b000101
#define DFSC_TRANS_FAULT_L2 0b000110
#define DFSC_TRANS_FAULT_L3 0b000111

#define DFSC_ACCESS_FAULT_L1 0b001001
#define DFSC_ACCESS_FAULT_L2 0b001010
#define DFSC_ACCESS_FAULT_L3 0b001011

#define DFSC_PERM_FAULT_L1 0b001101
#define DFSC_PERM_FAULT_L2 0b001110
#define DFSC_PERM_FAULT_L3 0b001111

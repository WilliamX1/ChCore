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

#define BEGIN_FUNC(_name)        \
        .global _name;           \
        .type _name, % function; \
        _name:

#define END_FUNC(_name) .size _name, .- _name

#define EXPORT(symbol) \
        .globl symbol; \
        symbol:

#define LOCAL_DATA(x) \
        .type x, 1;   \
        x:

#define DATA(x)    \
        .global x; \
        .hidden x; \
        LOCAL_DATA(x)

#define END_DATA(x) .size x, .- x

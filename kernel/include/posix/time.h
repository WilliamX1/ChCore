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

#include <posix/sys/types.h>

/* According to `https://pubs.opengroup.org/onlinepubs/9699919799/`. */

struct tm {
        int tm_sec; /* Seconds [0,60]. */
        int tm_min; /* Minutes [0,59]. */
        int tm_hour; /* Hour [0,23]. */
        int tm_mday; /* Day of month [1,31]. */
        int tm_mon; /* Month of year [0,11]. */
        int tm_year; /* Years since 1900. */
        int tm_wday; /* Day of week [0,6] (Sunday =0). */
        int tm_yday; /* Day of year [0,365]. */
        int tm_isds; /* Daylight Savings flag. */
};

struct timespec {
        time_t tv_sec; /* Seconds. */
        time_t tv_nsec; /* Nanoseconds. */
};

struct itimerspec {
        struct timespec it_interval; /* Timer period. */
        struct timespec it_value; /* Timer expiration. */
};

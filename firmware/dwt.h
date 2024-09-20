/* K155RT5 and K155RE3 programmer
 * Copyright (c) 2024 Aleksey Morozov aleksey.f.morozov@gmail.com aleksey.f.morozov@yandex.ru
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define SYSTEM_CORE_CLOCK 84000000u

static inline void dwt_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

#define US_TO_DWT(US) ((US) * (SYSTEM_CORE_CLOCK / 1000000u))

#define DWT_DELAY_US(T)                                             \
    do {                                                            \
        const uint32_t dwt_timeout = US_TO_DWT(T);                  \
        const uint32_t dwt_start = DWT->CYCCNT;                     \
        while ((uint32_t)(DWT->CYCCNT - dwt_start) < dwt_timeout) { \
        }                                                           \
    } while (0)

#define MS_TO_DWT(MS) ((MS) * (SYSTEM_CORE_CLOCK / 1000u))

#define DWT_DELAY_MS(T)                                             \
    do {                                                            \
        const uint32_t dwt_timeout = MS_TO_DWT(T);                  \
        const uint32_t dwt_start = DWT->CYCCNT;                     \
        while ((uint32_t)(DWT->CYCCNT - dwt_start) < dwt_timeout) { \
        }                                                           \
    } while (0)

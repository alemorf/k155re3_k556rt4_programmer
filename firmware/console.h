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

#include <stdio.h>

#define COLOR_LIGHT_RED "\x1B[0;1;8;31m"
#define COLOR_LIGHT_CYAN "\x1B[0;1;8;36m"
#define COLOR_DEFAULT "\x1B[0m"

static inline void print_error(const char *text) {
    printf(COLOR_LIGHT_RED "%s" COLOR_DEFAULT "\r\n", text);
}

void console(void);
void led_on(void);
void led_off(void);

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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void prog_select_k155re3(void);
void prog_select_k556rt4(void);
void prog_clear(void);
void prog_print(void);
bool prog_enter(const char *text);
void prog_read(void);
void prog_verify(void);
void prog_empty(void);
void prog_write(void);

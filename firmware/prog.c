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

#include "prog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "console.h"
#include "main.h"
#include "dwt.h"

static uint8_t prog_data_mask;
static unsigned prog_data_size;
static uint8_t prog_write_data[0x100];
static uint8_t prog_read_data[0x100];

static void io_set_address(uint8_t address) {
    GPIOA->ODR = address;
}

static void io_set_data(uint8_t data, bool write) {
    GPIOB->ODR = (~data & 0xFF) | (write ? 0 : 0x100);
}

static uint8_t io_get_data(void) {
    /* Data in: B9 B10 B12 B13 B14 B15 A8 A9 */
    const uint16_t a = GPIOA->IDR;
    const uint16_t b = GPIOB->IDR;
    return ((b >> 9) & 0x03) | ((b >> (12 - 2)) & (0x0F << 2)) | ((a >> (8 - 6)) & (0x03 << 6));
}

void prog_select_k155re3(void) {
    prog_data_mask = 0xFF;
    prog_data_size = 0x20;
    prog_clear();
}

void prog_select_k556rt4(void) {
    prog_data_mask = 0x0F;
    prog_data_size = 0x100;
    prog_clear();
}

static bool prog_is_chip_selected(void) {
    if (prog_data_size != 0)
        return true;
    print_error("Chip not selected");
    return false;
}

void prog_clear(void) {
    memset(prog_write_data, 0, sizeof(prog_write_data));
}

static void prog_print_internal(uint8_t *data) {
    static const unsigned bytes_per_line = 16;

    const char *format = prog_data_mask <= 0xF ? " %X" : " %02X";
    unsigned i;
    for (i = 0; i < prog_data_size; i += bytes_per_line) {
        printf("%02X:", i);
        unsigned j;
        for (j = 0; j < bytes_per_line && i + j < prog_data_size; j++)
            printf(format, data[i + j] & prog_data_mask);
        printf("\r\n");
    }
}

void prog_print(void) {
    if (!prog_is_chip_selected())
        return;

    prog_print_internal(prog_write_data);
}

bool prog_enter(const char *text) {
    char *end;
    unsigned long addr = strtoul(text, &end, 16);
    if (text == end || end[0] != ':')
        return false;

    if (!prog_is_chip_selected())
        return true;

    if (addr >= prog_data_size) {
        print_error("Too big address");
        return true;
    }

    const char *p = end + 1;
    for (;;) {
        while (*p == ' ')
            p++;
        if (*p == 0)
            return true;

        char *end;
        unsigned long byte = strtoul(p, &end, 16);
        if (p == end) {
            print_error("Incorrect number");
            return true;
        }
        p = end;

        if (byte > UINT8_MAX) {
            print_error("Incorrect number");
            return true;
        }

        if (addr >= prog_data_size) {
            print_error("Too big address");
            return true;
        }

        prog_write_data[addr] = byte;
        addr++;
    }
}

static void prog_read_internal(void) {
    unsigned i;
    for (i = 0; i < prog_data_size; i++) {
        io_set_address(i);
        DWT_DELAY_MS(1);
        prog_read_data[i] = io_get_data() & prog_data_mask;
    }
    io_set_address(0);
}

void prog_read(void) {
    if (!prog_is_chip_selected())
        return;

    prog_read_internal();

    prog_print_internal(prog_read_data);
}

void prog_verify(void) {
    if (!prog_is_chip_selected())
        return;

    prog_read_internal();

    unsigned i;
    for (i = 0; i < prog_data_size; i++) {
        if (((prog_read_data[i] ^ prog_write_data[i]) & prog_data_mask) != 0) {
            printf(COLOR_LIGHT_RED "Verify failed. %02X != %02X at %02X." COLOR_DEFAULT "\r\n",
                   prog_read_data[i] & prog_data_mask, prog_write_data[i] & prog_data_mask, i);
            return;
        }
    }

    printf("Verify done\r\n");
}

void prog_empty(void) {
    if (!prog_is_chip_selected())
        return;

    prog_read_internal();

    unsigned i;
    for (i = 0; i < prog_data_size; i++) {
        if ((prog_read_data[i] & prog_data_mask) != prog_data_mask) {
            printf(COLOR_LIGHT_RED "Not empty. %02X at %02X" COLOR_DEFAULT "\r\n", prog_read_data[i] & prog_data_mask,
                   i);
            return;
        }
    }

    printf("Empty\r\n");
}

void prog_write(void) {
    if (!prog_is_chip_selected())
        return;

    unsigned address;
    for (address = 0; address < prog_data_size; address++) {
        io_set_address(address);
        DWT_DELAY_MS(1);
        const uint8_t now = io_get_data();

        const uint8_t burn_bits = prog_write_data[address] & ~now & prog_data_mask;

        unsigned bit_number;
        for (bit_number = 0; bit_number < 8; bit_number++) {
            const uint8_t bit_mask = (1 << bit_number);
            if ((burn_bits & bit_mask) != 0) {
                printf("Burn %u bit to address %u\r\n", bit_number, address);

                /* Burn */
                io_set_data(bit_mask, false);
                DWT_DELAY_MS(300 * 3);  /* Скважность не менее 8 */
                led_on();
                __disable_irq();
                io_set_data(bit_mask, true);
                DWT_DELAY_MS(300 * 1);  /* Время программирования 300 мс */
                io_set_data(bit_mask, false);
                __enable_irq();
                led_off();
                DWT_DELAY_MS(300 * 4);     /* Скважность не менее 8 */
                io_set_data(0xFF, false);  /* Для чтения */

                /* Verify */
                if ((io_get_data() & bit_mask) == 0) {
                    io_set_address(0);
                    print_error("Burn failed");
                    return;
                }
            }
        }
    }

    prog_verify();
}

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

#include "console.h"
#include "main.h"
#include <stdlib.h>
#include <unistd.h>
#include "usbd_cdc_if.h"
#include "prog.h"
#include "dwt.h"

struct command {
    const char *name;
    void (*function)();
    bool empty;
    const char *help;
};

/* clang-format off */

static const struct command commands[] = {
    {"ADDR: BYTE BYTE BYTE...", NULL, false, "Enter data for write or verify"},
    {"re3", prog_select_k155re3, true, "Select K155RE3 chip"},
    {"rt4", prog_select_k556rt4, true, "Select K556RT4 chip"},
    {"clear", prog_clear, true, "Clear data (fill 0FFh)"},
    {"print", prog_print, true, "Print data"},
    {"read", prog_read, true, "Read data form K556PT4"},
    {"verify", prog_verify, true, "Read data from chip"},
    {"empty", prog_empty, true, "Check chip empty"},
    {"write", prog_write, true, "Write data to chip"},
    {NULL}
};

/* clang-format on */

extern USBD_HandleTypeDef hUsbDeviceFS;

static const char *skip_spaces(const char *p) {
    while (*p == ' ')
        p++;
    return p;
}

static void input(char *buf, size_t buf_size) {
    fflush(stdout);
    size_t pos = 0;
    for (;;) {
        char c;
        if (read(-0, &c, 1) == 0)
            continue;

        if (c == 0) {
            buf[0] = 0;
            return;
        }

        if (c == '\r' || c == '\n') {
            if (pos == 0)
                continue;
            break;
        }
        if (c == 8) {
            if (pos > 0) {
                write(0, "\x08 \x08", 3);
                pos--;
            }
            continue;
        }
        if (pos == buf_size - 1) {
            printf("\r\n" COLOR_LIGHT_RED "Line too long (%u)" COLOR_DEFAULT "\r\n", pos);
            pos = 0;
            continue;
        }
        write(0, &c, 1);
        buf[pos] = c;
        pos++;
    }
    write(0, "\r\n", 2);
    buf[pos] = 0;
}

static const char *parse_name(const char *p, const char *name) {
    const size_t name_len = strlen(name);
    if (0 != strncmp(p, name, name_len))
        return NULL;
    const char *p1 = p + name_len;
    if (*p1 != 0 && *p1 != ' ')
        return NULL;
    return skip_spaces(p1);
}

static void parse_command(const char *command, const struct command *c) {
    /* Skip spaces */
    const char *p = skip_spaces(command);

    /* Empty command */
    if (*p == 0)
        return;

    const struct command *i;
    for (i = c; i->name != NULL; i++) {
        if (i->function != NULL) {
            const char *p1 = parse_name(p, i->name);
            if (p1 != NULL) {
                if (i->empty && *p1 != 0) {
                    print_error("Extra characters at the end");
                    return;
                }
                i->function(p1);
                return;
            }
        }
    }

    /* ADDR: DATA DATA DATA */
    if (prog_enter(p))
        return;

    /* Show help */
    print_error("Unknown command");
    if (c->name != NULL) {
        printf(COLOR_LIGHT_CYAN "Help:\r\n");
        for (i = c; i->name != NULL; i++) {
            printf(i->name);
            printf("\t - ");
            printf(i->help);
            if (i[1].name == NULL)
                printf(COLOR_DEFAULT);
            printf("\r\n");
        }
    }
}

void led_on(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

void led_off(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void console(void) {
    dwt_init();
    for (;;) {
        led_on();

        while (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) {
        }

        DWT_DELAY_MS(1000);

        led_off();

        printf(COLOR_LIGHT_CYAN
               "K155RE3 and K556RT4 programmer\r\n"
               "(c) 20-09-2024 Aleksey Morozov aleksey.f.morozov@gmail.com" COLOR_DEFAULT "\r\n");

        while (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
            static char command[120];
            input(command, sizeof(command));
            parse_command(command, commands);
        }
    }
}

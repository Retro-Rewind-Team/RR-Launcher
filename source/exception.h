/*
    exception.h - exception stub handler definition

    Copyright (C) 2025  Retro Rewind Team

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef RRC_EXCEPTION_H
#define RRC_EXCEPTION_H

#include <gccore.h>

/* amount of time to display exception screen before exiting */
#define EXCEPTION_DISPLAY_TIME_SEC 10

/* offsets from EXCEPTION_HANDLER_BASE_ADDR */
#define EXCEPTION_HANDLER_STUB_OFFSETS { \
    0x0100,                              \
    0x0200,                              \
    0x0300,                              \
    0x0400,                              \
    0x0500,                              \
    0x0600,                              \
    0x0700,                              \
    0x0800,                              \
    0x0900,                              \
    0x0D00,                              \
    0x0F00,                              \
    0x1300,                              \
    0x1400,                              \
    0x1700,                              \
};

/* the instruction to use when de-initing (b 0x0) */
#define EXCEPTION_HANDLER_DEINIT_INSTR 0x48000000

#define EXCEPTION_HANDLERS_LEN 0x1f00

#define EXCEPTION_ASM_REGS_ADDR 0x80003500

void init_exception_handlers();

/* we need to de-init these before patching because weirdness may happen - our
   exception handler may get overwritten during patching so replace all of them
   with a simple branch to self (hang) */
void deinit_exception_handlers();

#endif
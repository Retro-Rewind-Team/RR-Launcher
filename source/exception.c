/*
    exception.h - exception stub handler

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

/*
    Some of this code is adapted from the Homebrew channel and is licenced
    differently - specifically, `void _handle_exception(int exception)' and
    void `init_exception_handlers()'.
    See: https://github.com/fail0verflow/hbc/blob/master/wiipax/stub/exception.c

    Copyright (C) fail0verflow/Team Twiizers and all contributors. Licensed under
    GPL-2-or-later. See <https://www.gnu.org/licenses/> for license details.
*/

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "exception.h"
#include "console.h"

extern char exception_catch_start, exception_catch_end;

void _handle_exception(int exception)
{
    rrc_con_clear(false);
    rrc_con_cursor_seek_to(0, 0);
    printf("test...");
    usleep(EXCEPTION_DISPLAY_TIME_SEC * 1000 * 1000);
    /* call lower level exit (doesnt call exit handlers) to minimise amount of work
       this handler does */
    _exit(-1);
}

void init_exception_handlers()
{
    u32 offsets[] = EXCEPTION_HANDLER_STUB_OFFSETS;

    for (u32 i = 0; i < sizeof(offsets) / sizeof(u32); i++)
    {
        u32 *instr = (u32 *)(0x80000000 + offsets[i]);

        /* set up and branch to exception catch asm */
        /* stmw 0,0x3500(0) (copy contents of all GPRs to EXCEPTION_ASM_REGS_ADDR) -
           this is where registers are stored for display in the handler */
        instr[0] = 0xbc003500;
        instr[1] = 0x38600000 | i; // li 3,i (load vector value into general purpose register 3)
        instr[2] = 0x48003602;     // ba 0x3600 (branch to asm based on EXCEPTION_ASM_ADDR)
        instr[3] = 0;
    }

    u32 asm_len = &exception_catch_end - &exception_catch_start;
    void *ecs = &exception_catch_start;
    /* place exception handler asm early in instruction memory */
    memcpy((void *)0x80003600, &ecs, asm_len);
    ICInvalidateRange((void *)0x80003600, asm_len);
}

void deinit_exception_handlers()
{
    u32 offsets[] = EXCEPTION_HANDLER_STUB_OFFSETS;

    for (u32 i = 0; i < sizeof(offsets) / sizeof(u32); i++)
    {
        *(u32 *)(0x80000000 + offsets[i]) = EXCEPTION_HANDLER_DEINIT_INSTR;
    }

    ICInvalidateRange((u32 *)0x80000000, EXCEPTION_HANDLERS_LEN);
}
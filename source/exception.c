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
#include <stdlib.h>
#include <gccore.h>
#include <ogc/wiilaunch.h>

#include "exception.h"
#include "console.h"
#include "gui.h"

extern char exception_catch_start, exception_catch_end;

/* https://wiibrew.org/wiki/Revolution_OS#Exceptions
   more are defined here than we actually handle */
char *_exception_to_type(int exception)
{
    switch (exception)
    {
    case 0x100:
        return "System Reset";
    case 0x200:
        return "Machine Check";
    case 0x300:
        return "DSI";
    case 0x400:
        return "ISI";
    case 0x500:
        return "IRQ";
    case 0x600:
        return "Alignment";
    case 0x700:
        return "Program";
    case 0x800:
        return "FP unavailable";
    case 0x900:
        return "Decrementer";
    case 0xC00:
        return "Syscall";
    case 0xD00:
        return "Trace";
    case 0xF00:
        return "Performance";
    case 0x1300:
        return "IABR";
    case 0x1400:
        return "Reserved";
    case 0x1700:
        return "Thermal";
    default:
        return "Unknown";
    }
}

void _handle_exception(int exception)
{
    u32 *x;
    u32 i;
    rrc_con_clear(false);
    rrc_con_cursor_seek_to(0, 0);
    char *type = _exception_to_type(exception);

    printf(RRC_CON_ANSI_FG_BRIGHT_WHITE "\n%s exception occurred! Reloading in %i secs...\n", type, EXCEPTION_DISPLAY_TIME_SEC);

    x = (u32 *)0x80003500;

    printf(RRC_CON_ANSI_FG_BRIGHT_WHITE "\n R0..R7    R8..R15  R16..R23  R24..R31\n");
    for (i = 0; i < 8; i++)
    {
        printf(RRC_CON_ANSI_FG_WHITE "%08x  %08x  %08x  %08x\n", x[0], x[8], x[16], x[24]);
        x++;
    }
    x += 24;

    printf(RRC_CON_ANSI_FG_BRIGHT_WHITE "\n CR/XER    LR/CTR  SRR0/SRR1 DAR/DSISR\n");
    for (i = 0; i < 2; i++)
    {
        printf(RRC_CON_ANSI_FG_WHITE "%08x  %08x  %08x  %08x\n", x[0], x[2], x[4], x[6]);
        x++;
    }

    usleep(EXCEPTION_DISPLAY_TIME_SEC * 1000 * 1000);

    for (;;)
        _exit(-1);
}

void sync_before_exec(const void *p, u32 len)
{
    u32 a, b;

    a = (u32)p & ~0x1f;
    b = ((u32)p + len + 0x1f) & ~0x1f;

    for (; a < b; a += 32)
        asm("dcbst 0,%0 ; sync ; icbi 0,%0" : : "b"(a));

    asm("sync ; isync");
}

void init_exception_handlers()
{
    u32 vector;
    u32 len_asm;

    for (vector = 0x100; vector < 0x800; vector += 0x10)
    {
        /* ingore IRQs */
        if (vector < 0x500 || vector >= 0x600)
        {
            u32 *instr = (u32 *)(0x80000000 + vector);

            /* set up and branch to exception catch asm */
            /* stmw 0,0x3500(0) (copy contents of all GPRs to EXCEPTION_ASM_REGS_ADDR) -
               this is where registers are stored for display in the handler */
            instr[0] = 0xbc003500;
            instr[1] = 0x38600000 | vector; // li 3,i (load vector value into general purpose register 3)
            instr[2] = 0x48003602;          // ba 0x3600 (branch to asm based on EXCEPTION_ASM_ADDR)
            instr[3] = 0;
        }
    }

    sync_before_exec((void *)0x80000100, 0x1f00);

    len_asm = &exception_catch_end - &exception_catch_start;
    memcpy((void *)0x80003600, &exception_catch_start, len_asm);
    sync_before_exec((void *)0x80003600, len_asm);
}

void deinit_exception_handlers()
{
    u32 vector;

    for (vector = 0x200; vector < 0x800; vector += 0x10)
    {
        u32 *instr = (u32 *)(0x80000000 + vector);

        instr[0] = 0x48000000;
    }

    sync_before_exec((void *)0x80000100, 0x1f00);
}
/*
    util.h - Utility function declarations and macros.

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

#ifndef RRC_RUNTIME_EXT_UTIL
#define RRC_RUNTIME_EXT_UTIL

#include <types.h>

void OS_Report(const char *, ...);
void OS_Fatal(u32 *, u32 *, const char *);

#ifndef DEBUG
#define RTE_DBG(...)
#endif

#ifdef DEBUG
#define RTE_DBG OS_Report
#endif

#define RTE_FATAL(...)                           \
    do                                           \
    {                                            \
        u32 fg = 0xFFFFFFFF, bg = 0;             \
        char buf[128];                           \
        snprintf(buf, sizeof(buf), __VA_ARGS__); \
        OS_Fatal(&fg, &bg, buf);                 \
        while (1)                                \
            ;                                    \
    } while (0);

#define RTE_STRINGIFY(x) #x

u32 align_down(u32 num, u32 align_as);
u32 align_up(u32 num, u32 align_as);

#endif

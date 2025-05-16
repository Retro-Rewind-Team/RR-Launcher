/*
    dol.h - definition of DOL (static executable) structure

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

/* See: https://wiki.tockdom.com/wiki/DOL_(File_Format) */

#ifndef RRC_DOL_H
#define RRC_DOL_H

#include <gctypes.h>

#define RRC_DOL_SECTION_COUNT 18

struct rrc_dol
{
    u32 section[RRC_DOL_SECTION_COUNT];
    u32 section_addr[RRC_DOL_SECTION_COUNT];
    u32 section_size[RRC_DOL_SECTION_COUNT];

    u32 bss_addr;
    u32 bss_size;
    u32 entry_point;
};

#endif

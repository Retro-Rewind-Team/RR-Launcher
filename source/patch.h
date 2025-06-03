/*
    patch.h - final game launching logic (see patch.c for details)

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

#ifndef RRC_PATCH_H
#define RRC_PATCH_H

#include <gctypes.h>
#include <dol.h>
#include <riivo.h>

void patch_dol(
    struct rrc_dol *dol,
    struct rrc_riivo_memory_patch *mem_patches,
    int mem_patch_count,
    void (*ic_invalidate_range)(void *, u32),
    void (*dc_flush_range)(void *, u32));

#endif

/*
    binary_loader.h - loading of misc binaries

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

#ifndef BINARY_LOADER_H
#define BINARY_LOADER_H

#include <dol.h>
#include "../result.h"

#define RRC_LOADER_PUL_PATH "RetroRewind6/Binaries/Loader.pul"

// We need to load the correct runtime-ext.
// This is provided as a base; however, the region and file extension needs
// to be appended at runtime.
#define RRC_RUNTIME_EXT_BASE_PATH "RetroRewindChannel/runtime-ext"

// At the moment this file handles Loader.pul and runtime-ext.

/**
 * Finds a game section that contains the given address.
 * Sets `virt_addr` to the address of `addr` within the section in the DOL in safe space,
 * and sets `section_index` to the index of the section.
 */
bool rrc_binary_find_section_by_addr(struct rrc_dol *dol, u32 addr, void **virt_addr, u32 *section_index);

struct rrc_result rrc_binary_load_pulsar_loader(struct rrc_dol *dol, void *real_loader_addr);

struct rrc_result rrc_binary_load_runtime_ext(char region);

#endif
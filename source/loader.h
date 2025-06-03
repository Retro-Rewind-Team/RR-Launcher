/*
    loader.h - main app loader implementation header

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

#ifndef RRC_LOADER_H
#define RRC_LOADER_H

#include "settingsfile.h"
#include <dol.h>

#define RRC_BI2_SIZE 0x2000
#define RRC_PATCH_COPY_ADDRESS 0x80900000
// Must be kept in sync with the .riivo_disc_ptr section address in runtime-ext's linker script
#define RRC_RIIVO_DISC_PTR 0x81782fa0

#define RRC_DVD_CONVERT_PATH_TO_ENTRYNUM 0x8015df4c
#define RRC_DVD_FAST_OPEN 0x8015e254
#define RRC_DVD_OPEN 0x8015e2bc
#define RRC_DVD_READ_PRIO 0x8015e834
#define RRC_DVD_CLOSE 0x8015e568

#define RRC_LOADER_PUL_PATH "RetroRewind6/Binaries/Loader.pul"
#define RRC_RUNTIME_EXT_PATH "RetroRewindChannel/runtime-ext.dol"

/*
 * Spins until Mario Kart Wii is inserted into the disc drive.
 *
 * Returns normal RRC status codes.
 */
int rrc_loader_await_mkw(void *xfb);

/*
 * Locate the data partition and return it. On failure, `part' is set to NULL.
 */
int rrc_loader_locate_data_part(u32 *part);

/*
 * This routine applies all patches from code.pul as well as setting key memory addresses
 * appropriately before fully loading the DOL and launching Mario Kart Wii.
 *
 * This function should always return a status code on failure and NEVER CRASH. On success, it never returns.
 */
void rrc_loader_load(struct rrc_dol *dol, struct rrc_settingsfile *settings, void *bi2_dest, u32 mem1_hi, u32 mem2_hi);

#endif

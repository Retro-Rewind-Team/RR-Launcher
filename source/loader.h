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

/*
 * Spins until Mario Kart Wii is inserted into the disc drive.
 *
 * Returns normal RRC exit codes.
 */
int rrc_loader_await_mkw();

/*
 * Locate the data partition and return it. On failure, `part' is set to NULL.
 */
int rrc_loader_locate_data_part(u32 *part);

/*
 * This routine applies all patches from code.pul as well as setting key memory addresses
 * appropriately before fully loading the DOL and launching Mario Kart Wii.
 *
 * TODO: For now, this does not apply patches, but we'll want to do that.
 * TODO: This routine should be resilient against things such as the SD card or disc being ejected.
 * This function should always return a status code on failure and NEVER CRASH. On success, it never returns.
 */
int rrc_loader_load();

#endif
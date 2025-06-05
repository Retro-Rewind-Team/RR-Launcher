/*
    disc_loader.h - disc loading and identification

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

#ifndef DISC_LOADER_H
#define DISC_LOADER_H

#include <gccore.h>

/*
 * Spins until Mario Kart Wii is inserted into the disc drive.
 *
 * The region pointer is populated with the disc's region. This is needed to
 * load the patches at appropriate addresses.
 *
 * Returns normal RRC status codes.
 */
int rrc_disc_loader_await_mkw(void *xfb, char *region);

/*
 * Locate the data partition and return it. On failure, `part' is set to NULL.
 */
int rrc_disc_loader_locate_data_part(u32 *part);

#endif
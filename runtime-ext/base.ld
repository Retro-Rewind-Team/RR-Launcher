/*
    base.ld - Base linker script independent of regions

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

SECTIONS {
    # .init, .text and .rodata are specified in Makefile LDFLAGS
    .sd_vtable                    0x81782e00 : { *(.sd_vtable) }
    .dvd_convert_path_to_entrynum 0x81782e60 : { *(.dvd_convert_path_to_entrynum) }
    .dvd_open                     0x81782ea0 : { *(.dvd_open) }
    .dvd_fast_open                0x81782ee0 : { *(.dvd_fast_open) }
    .dvd_read_prio                0x81782f20 : { *(.dvd_read_prio) }
    .dvd_close                    0x81782f60 : { *(.dvd_close) }
    .riivo_disc_ptr               0x81782fa0 : { *(.riivo_disc_ptr) }
}

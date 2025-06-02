/*
    main.c - Main entry point for the runtime DOL

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

#include <stdbool.h>
#include <types.h>
#include <fcntl.h>
#include <string.h>
#include <io/fat-sd.h>
#include <rvl/cache.h>
#include <libfat/fatfile.h>
#include <riivo.h>
#include <dol.h>
#include "util.h"
#include "dvd.h"

#define EXPORT_FUNCTION(secname, decl_args, call_args, implname) \
    __attribute__((section(secname))) s32 __##implname decl_args \
    {                                                            \
        return implname call_args;                               \
    }

EXPORT_FUNCTION(".dvd_convert_path_to_entrynum", (const char *path), (path), custom_convert_path_to_entry_num_impl);
EXPORT_FUNCTION(".dvd_open", (const char *path, FileInfo *file_info), (path, file_info), custom_open_impl);
EXPORT_FUNCTION(".dvd_fast_open", (s32 entry_num, FileInfo *file_info), (entry_num, file_info), custom_fast_open_impl);
EXPORT_FUNCTION(".dvd_read_prio", (FileInfo * file_info, void *buffer, s32 length, s32 offset, s32 prio), (file_info, buffer, length, offset, prio), custom_read_prio_impl);
EXPORT_FUNCTION(".dvd_close", (FileInfo * file_info), (file_info), custom_close_impl);

int _start()
{
    // Prevent linker from DCE'ing the functions
    *(volatile u32 *)__custom_convert_path_to_entry_num_impl;
    *(volatile u32 *)__custom_open_impl;
    *(volatile u32 *)__custom_fast_open_impl;
    *(volatile u32 *)__custom_read_prio_impl;
    *(volatile u32 *)__custom_close_impl;

    // Get the compiler to remove all unnecessary libogc deinitialization code, this function is never actually called
    while (1)
        ;
}

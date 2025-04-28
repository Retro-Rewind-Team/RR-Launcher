/*
    util.h - utility function implementations

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

#include <gctypes.h>
#include <sys/statvfs.h>
#include "result.h"

u32 align_down(u32 num, u32 align_as)
{
    return num & -align_as;
}

u32 align_up(u32 num, u32 align_as)
{
    return (num + align_as - 1) & -align_as;
}

struct rrc_result sd_get_free_space(unsigned long *res)
{
    struct statvfs sbx;
    int rr = statvfs("/dev/sd", &sbx);
    if (rr != 0)
    {
        return rrc_result_create_error_errno(errno, "Failed to get free space on SD card");
    }

    *res = sbx.f_bavail * sbx.f_frsize;
    return rrc_result_success;
}

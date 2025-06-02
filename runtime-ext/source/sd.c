/*
    sd.c - SD helper functions.

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
#include <io/fat.h>
#include <io/fat-sd.h>
#include <stdio.h>
#include <errno.h>
#include "util.h"

s32 rrc_rt_sd_init()
{
    static bool mounted = false;
    if (!mounted)
    {
        int res = SD_Mount();
        if (res != 0)
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "SD_Mount failed: %d (errno:%d)\n", res, errno);
            RTE_FATAL(buf);
        }
        mounted = true;
        res = SD_chdir("sd:/");
        if (res != 0)
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "SD_chdhir failed: %d (errno:%d)\n", res, errno);
            RTE_FATAL(buf);
        }
        return res;
    }
    return 0;
}

bool rrc_rt_sd_file_exists(const char *path)
{
    FILE_STRUCT fs;
    s32 tmpfd = SD_open(&fs, path, O_RDONLY);
    if (tmpfd != -1)
    {
        RTE_DBG("DEBUG: File size of %s: %d\n", path, fs.filesize);
        SD_close(tmpfd);
        return true;
    }

    return false;
}

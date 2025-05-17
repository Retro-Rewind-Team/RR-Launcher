/*
    sd.h - SD card helper and initialisation routines

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

#include <fat.h>
#include "unistd.h"

#include "sd.h"

struct rrc_result rrc_sd_init()
{
    if (!fatInitDefault())
    {
        struct rrc_result sdfail = {
            .errtype = ESOURCE_SD_CARD,
            .context = "Couldn't mount the SD card - is it inserted?",
            .inner = {
                .errnocode = EIO}};

        return sdfail;
    }

    if (chdir("sd:/") == -1)
    {
        return rrc_result_create_error_errno(errno, "Failed to set SD card root");
    }

    FILE *file = fopen(RRC_SD_TEST_FILE, "w");
    if (!file)
    {
        return rrc_result_create_error_errno(errno, "The SD card is not writeable. Make sure it is unlocked.");
    }
    fclose(file);
    unlink(RRC_SD_TEST_FILE);

    return rrc_result_success;
}

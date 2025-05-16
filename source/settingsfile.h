/*
    settingsfile.h - API for interacting with the settings file that stores the selected settings
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

#ifndef RRC_SETTINGSFILE_H
#define RRC_SETTINGSFILE_H

#include <gctypes.h>
#include "result.h"

#define RRC_SETTINGSFILE_DEFAULT 0            /* disabled */
#define RRC_SETTINGSFILE_AUTOUPDATE_DEFAULT 1 /* enabled */
#define RRC_SETTINGSFILE_PACK_ENABLED_VALUE 1

enum rrc_settingsfile_status
{
    /** Operation was sucessful */
    RRC_SETTINGSFILE_OK,
    /** Failed to open/create settings file */
    RRC_SETTINGS_FILE_FOPEN
};

struct rrc_settingsfile
{
    u32 my_stuff;
    u32 language;
    u32 savegame;
    u32 auto_update;
};

/**
 * Creates a settingsfile on the SD card.
 */
struct rrc_result rrc_settingsfile_create();

/**
 * Initializes an `rrc_settingsfile` by reading it from the sd card.
 * If it does not already exist, this function will create it and initialize the file with default values.
 * The settings pointee will always be fully initialized, even in case of an error (in which case it will have default values).
 */
struct rrc_result rrc_settingsfile_parse(struct rrc_settingsfile *settings);

/**
 * Writes an `rrc_settingsfile` to the sd card.
 */
struct rrc_result rrc_settingsfile_store(struct rrc_settingsfile *settings);

#endif

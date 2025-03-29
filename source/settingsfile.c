/*
    settingsfile.c - implementation of the settings file
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

/*
    Currently the file format is defined as follows:

    | Name              | Size in bytes      |
    |-------------------|--------------------|
    | Format Magic      | 4                  | (always the value of `RRC_SETTINGSFILE_MAGIC`)
    | Format Version    | 4                  |
    | Number of Entries | 4                  |
    | List of Entries   | Variable           |


    Entry Format:
    | Name            | Size in bytes (BE) |
    |-----------------|--------------------|
    | Key Name Length | 4                  |
    | Key Name        | Variable           |
    | Value Length    | 4                  |
    | Value           | Variable           |
*/

#include <stdio.h>
#include <gctypes.h>
#include <string.h>
#include <errno.h>
#include "settingsfile.h"
#include "util.h"

#define RRC_SETTINGSFILE_PATH "RetroRewind6/.settings"
#define RRC_SETTINGSFILE_MY_STUFF_KEY "My Stuff"
#define RRC_SETTINGSFILE_LANGUAGE_KEY "Language"
#define RRC_SETTINGSFILE_SAVEGAME_KEY "Separate savegame"
#define RRC_SETTINGSFILE_MAGIC 1920234103
#define RRC_SETTINGSFILE_VERSION 0

static u32 expect_read_u32(FILE *file, const char *what)
{
    u32 val;
    RRC_ASSERTEQ(fread(&val, sizeof(u32), 1, file), 1, what);
    return val;
}

static void rrc_settings_file_write_header(FILE *file, u32 entry_count)
{
    u32 magic = RRC_SETTINGSFILE_MAGIC;
    RRC_ASSERTEQ(fwrite(&magic, sizeof(magic), 1, file), 1, "write magic");

    u32 version = RRC_SETTINGSFILE_VERSION;
    RRC_ASSERTEQ(fwrite(&version, sizeof(version), 1, file), 1, "write version");

    RRC_ASSERTEQ(fwrite(&entry_count, sizeof(entry_count), 1, file), 1, "write entry count");
}

FILE *rrc_settingsfile_create()
{
    FILE *file = fopen(RRC_SETTINGSFILE_PATH, "w");
    if (file)
    {
        rrc_settings_file_write_header(file, 0);
        fseek(file, 0, SEEK_SET);
    }
    return file;
}

enum rrc_settingsfile_status rrc_settingsfile_parse(struct rrc_settingsfile *settings)
{
    FILE *file = fopen(RRC_SETTINGSFILE_PATH, "r");
    if (!file)
    {
        if (errno == ENOENT)
        {
            // File doesn't exist. Create it and initialize it with defaults.
            file = rrc_settingsfile_create();
            if (!file)
            {
                return RRC_SETTINGS_FILE_FOPEN;
            }
        }
        else
        {
            return RRC_SETTINGS_FILE_FOPEN;
        }
    }

    RRC_ASSERTEQ(expect_read_u32(file, "read file header"), RRC_SETTINGSFILE_MAGIC, "magic header mismatch");
    expect_read_u32(file, "read file version"); // Version unused for now.

    u32 entry_count = expect_read_u32(file, "read entry count");

    settings->my_stuff = RRC_SETTINGSFILE_DEFAULT;
    settings->language = RRC_SETTINGSFILE_DEFAULT;
    settings->savegame = RRC_SETTINGSFILE_DEFAULT;

    for (int i = 0; i < entry_count; i++)
    {
        // Read key length.
        u32 key_length = expect_read_u32(file, "read key length");

        RRC_ASSERT(key_length < 32, "setting keys should not be longer than 32 characters");

        // Read the key.
        char key[32];
        int read = fread((void *)key, sizeof(char), key_length, file);
        RRC_ASSERT(read == key_length, "failed to read key");
        key[key_length] = 0;

        // Read value length. For now we always have u32 values.
        u32 value_length = expect_read_u32(file, "read value length");
        RRC_ASSERTEQ(value_length, 4, "value should always be a u32");

        // Read the value. Currently this is always a u32.
        u32 value;
        read = fread((void *)&value, sizeof(u32), 1, file);
        RRC_ASSERT(read == 1, "failed to read a u32 value");

        if (strcmp(key, "My Stuff") == 0)
        {
            settings->my_stuff = value;
        }
        else if (strcmp(key, "Language") == 0)
        {
            settings->language = value;
        }
        else if (strcmp(key, "Separate savegame") == 0)
        {
            settings->savegame = value;
        }
    }

    fclose(file);
    return RRC_SETTINGSFILE_OK;
}

static void rrc_settingsfile_set_option(FILE *file, const char *key, u32 value)
{
    u32 key_len = strlen(key);
    RRC_ASSERTEQ(fwrite(&key_len, sizeof(key_len), 1, file), 1, "write len");
    RRC_ASSERTEQ(fwrite(key, 1, key_len, file), key_len, "write key");

    u32 val_len = 4; // always a u32
    RRC_ASSERTEQ(fwrite(&val_len, sizeof(val_len), 1, file), 1, "write value length");
    RRC_ASSERTEQ(fwrite(&value, sizeof(value), 1, file), 1, "write value");
}

/**
 * Writes an `rrc_settingsfile` to the sd card.
 */
enum rrc_settingsfile_status rrc_settingsfile_store(struct rrc_settingsfile *settings)
{
    FILE *file = fopen(RRC_SETTINGSFILE_PATH, "w");
    RRC_ASSERT(file != NULL, "failed to open file");

    rrc_settings_file_write_header(file, 3);

    rrc_settingsfile_set_option(file, RRC_SETTINGSFILE_MY_STUFF_KEY, settings->my_stuff);
    rrc_settingsfile_set_option(file, RRC_SETTINGSFILE_LANGUAGE_KEY, settings->language);
    rrc_settingsfile_set_option(file, RRC_SETTINGSFILE_SAVEGAME_KEY, settings->savegame);

    fclose(file);
    return RRC_SETTINGSFILE_OK;
}

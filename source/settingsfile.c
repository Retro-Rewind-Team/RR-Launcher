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

#define RRC_SETTINGSFILE_PATH "RetroRewindChannel/.settings"
#define RRC_SETTINGSFILE_MY_STUFF_KEY "My Stuff"
#define RRC_SETTINGSFILE_LANGUAGE_KEY "Language"
#define RRC_SETTINGSFILE_SAVEGAME_KEY "Separate savegame"
#define RRC_SETTINGSFILE_AUTOUPDATE_KEY "Auto update"
#define RRC_SETTINGSFILE_MAGIC 1920234103
#define RRC_SETTINGSFILE_VERSION 0

struct rrc_result expect_read_u32(FILE *file, u32 *val, const char *what)
{
    int read = fread(val, sizeof(u32), 1, file);
    if (read == 1)
    {
        return rrc_result_success;
    }
    else
    {
        return rrc_result_create_error_corrupted_settingsfile(what);
    }
}

static struct rrc_result rrc_settingsfile_write_header(FILE *file, u32 entry_count)
{
    u32 magic = RRC_SETTINGSFILE_MAGIC;
    if (fwrite(&magic, sizeof(magic), 1, file) != 1)
    {
        return rrc_result_create_error_errno(errno, "Failed to write magic header");
    }

    u32 version = RRC_SETTINGSFILE_VERSION;
    if (fwrite(&version, sizeof(version), 1, file) != 1)
    {
        return rrc_result_create_error_errno(errno, "Failed to write version");
    }

    if (fwrite(&entry_count, sizeof(entry_count), 1, file) != 1)
    {
        return rrc_result_create_error_errno(errno, "Failed to write entry count");
    }

    return rrc_result_success;
}

struct rrc_result rrc_settingsfile_create()
{
    FILE *file = fopen(RRC_SETTINGSFILE_PATH, "w");
    if (!file)
    {
        return rrc_result_create_error_errno(errno, "could not create settingsfile, maybe the sd card is locked?");
    }

    rrc_settingsfile_write_header(file, 0);
    fclose(file);

    return rrc_result_success;
}

void rrc_settingsfile_init_defaults(struct rrc_settingsfile *settings)
{
    settings->my_stuff = RRC_SETTINGSFILE_DEFAULT;
    settings->language = RRC_SETTINGSFILE_DEFAULT;
    settings->savegame = RRC_SETTINGSFILE_DEFAULT;
    settings->auto_update = RRC_SETTINGSFILE_AUTOUPDATE_DEFAULT;
}

struct rrc_result rrc_settingsfile_parse(struct rrc_settingsfile *settings)
{
    // Initialize it with defaults early so that it will always at least have valid values even in case of an error.
    rrc_settingsfile_init_defaults(settings);

    FILE *file = fopen(RRC_SETTINGSFILE_PATH, "r");
    if (!file && errno == ENOENT)
    {
        // File doesn't exist. Create it and initialize it with default values.
        TRY(rrc_settingsfile_create());

        file = fopen(RRC_SETTINGSFILE_PATH, "r");
    }

    if (!file)
    {
        // Either an error other than ENOENT happened or we still fail to open after having created the file.
        // Either way, it's unclear what exactly went wrong here...
        return rrc_result_create_error_errno(errno, "failed to open file for reading after creating it");
    }

    u32 magic;
    TRY(expect_read_u32(file, &magic, "Failed to read magic bytes"));
    if (magic != RRC_SETTINGSFILE_MAGIC)
    {
        return rrc_result_create_error_corrupted_settingsfile("Magic header mismatch");
    }
    u32 version;
    TRY(expect_read_u32(file, &version, "Failed to read version number")); // Version unused for now.

    u32 entry_count;
    TRY(expect_read_u32(file, &entry_count, "Failed to read entry count"));

    for (int i = 0; i < entry_count; i++)
    {
        // Read key length.
        u32 key_length;
        TRY(expect_read_u32(file, &key_length, "Failed to read length of key"));

        if (key_length >= 32)
        {
            return rrc_result_create_error_corrupted_settingsfile("Settings key cannot be longer than 32 characters");
        }

        // Read the key.
        char key[32];
        int read = fread(key, sizeof(char), key_length, file);
        if (read != key_length)
        {
            return rrc_result_create_error_corrupted_settingsfile("Failed to fully read key");
        }

        key[key_length] = 0;

        // Read value length. For now we always have u32 values.
        u32 value_length;
        TRY(expect_read_u32(file, &value_length, "Failed to read length of value"));
        if (value_length != 4)
        {
            return rrc_result_create_error_corrupted_settingsfile("Value length currently should always be 4 (u32)");
        }

        // Read the value. Currently this is always a u32.
        u32 value;
        read = fread(&value, sizeof(u32), 1, file);
        if (read != 1)
        {
            return rrc_result_create_error_corrupted_settingsfile("Failed to fully read u32 value");
        }

        if (strcmp(key, RRC_SETTINGSFILE_MY_STUFF_KEY) == 0)
        {
            settings->my_stuff = value;
        }
        else if (strcmp(key, RRC_SETTINGSFILE_LANGUAGE_KEY) == 0)
        {
            settings->language = value;
        }
        else if (strcmp(key, RRC_SETTINGSFILE_SAVEGAME_KEY) == 0)
        {
            settings->savegame = value;
        }
        else if (strcmp(key, RRC_SETTINGSFILE_AUTOUPDATE_KEY) == 0)
        {
            settings->auto_update = value;
        }
    }

    fclose(file);
    return rrc_result_success;
}

struct rrc_result rrc_settingsfile_set_option(FILE *file, const char *key, u32 value)
{
    u32 key_len = strlen(key);
    if (fwrite(&key_len, sizeof(key_len), 1, file) != 1)
    {
        return rrc_result_create_error_errno(errno, "Failed to write key length");
    }

    if (fwrite(key, 1, key_len, file) != key_len)
    {
        return rrc_result_create_error_errno(errno, "Failed to write key");
    }

    u32 val_len = 4; // always a u32
    if (fwrite(&val_len, sizeof(val_len), 1, file) != 1)
    {
        return rrc_result_create_error_errno(errno, "Failed to write value length");
    }

    if (fwrite(&value, sizeof(value), 1, file) != 1)
    {
        return rrc_result_create_error_errno(errno, "Failed to write value");
    }

    return rrc_result_success;
}

/**
 * Writes an `rrc_settingsfile` to the sd card.
 */
struct rrc_result rrc_settingsfile_store(struct rrc_settingsfile *settings)
{
    FILE *file = fopen(RRC_SETTINGSFILE_PATH, "w");
    if (file == NULL)
    {
        return rrc_result_create_error_errno(errno, "Failed to open settingsfile");
    }

    rrc_settingsfile_write_header(file, 4);

    TRY(rrc_settingsfile_set_option(file, RRC_SETTINGSFILE_MY_STUFF_KEY, settings->my_stuff));
    TRY(rrc_settingsfile_set_option(file, RRC_SETTINGSFILE_LANGUAGE_KEY, settings->language));
    TRY(rrc_settingsfile_set_option(file, RRC_SETTINGSFILE_SAVEGAME_KEY, settings->savegame));
    TRY(rrc_settingsfile_set_option(file, RRC_SETTINGSFILE_AUTOUPDATE_KEY, settings->auto_update));

    fclose(file);
    return rrc_result_success;
}

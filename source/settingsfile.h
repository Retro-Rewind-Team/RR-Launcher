#ifndef RRC_SETTINGSFILE_H
#define RRC_SETTINGSFILE_H

#define RRC_SETTINGSFILE_DEFAULT 0 /* disabled */

enum rrc_settingsfile_status
{
    /** Operation was sucessful */
    RRC_SETTINGSFILE_OK,
    /** Failed to open/create settings file */
    RRC_SETTINGS_FILE_FOPEN
};

struct rrc_settingsfile
{
    int my_stuff;
    int language;
    int savegame;
};

/**
 * Initializes an `rrc_settingsfile` by reading it from the sd card.
 * If it does not already exist, this function will create it and initialize the file with default values.
 */
enum rrc_settingsfile_status rrc_settingsfile_parse(struct rrc_settingsfile *settings);

/**
 * Writes an `rrc_settingsfile` to the sd card.
 */
enum rrc_settingsfile_status rrc_settingsfile_store(struct rrc_settingsfile *settings);

#endif

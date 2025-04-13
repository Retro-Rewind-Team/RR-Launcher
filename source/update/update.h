/*
    update.h - distribution update headers
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

#ifndef RRC_UPDATE_H
#define RRC_UPDATE_H

#include <curl/curl.h>
#include "../result.h"

#define RRC_UPDATE_LARGE_THRESHOLD (long)(1000 * 1000 * 100) /* 100MB */
#define RRC_VERSIONFILE "RetroRewind6/version.txt"

/* Holds all info related to an update or sequence of updates */
struct rrc_update_state
{
    /* Amount of updates needed */
    int num_updates;
    /* The current update */
    int current_update_num;
    /* Pointer to current downloaded data. Resets at the start of each download */
    char *d_ptr;
    /* All URLs for updates, in order. Length = num_updates */
    char **update_urls;
    /* Version of each update. Has the same length as `update_urls` and each index into update_urls is also valid for update_versions */
    int *update_versions;
    /* The current version. */
    int current_version;
    /* Amount of files to delete. */
    int num_deleted_files;
    /* Files to delete. */
    struct rrc_versionsfile_deleted_file *deleted_files;
};

/*
    Returns an int specifying version information from version.txt.
    E.g., 4.2.0 = 420
    SD driver must be loaded for this to work.
*/
struct rrc_result rrc_update_get_current_version(int *version);

/*
    Writes the specified version int into version.txt.
    SD driver must be loaded for this to work.
*/
struct rrc_result rrc_update_set_current_version(int version);

/*
    Downloads a Retro Rewind ZIP. Uses the console to display progress.
    Stores on SD in the file given by `filename'.
*/
struct rrc_result rrc_update_download_zip(char *url, char *filename, int current_zip, int max_zips);

enum rrc_update_ecode
{
    /* Success */
    RRC_UPDATE_EOK = 0,

    /* CURL error. `ccode' is set to that error if this is set. */
    RRC_UPDATE_ECURL,

    /* IO Errors */
    /* Could not open file */
    RRC_UPDATE_INVFILE,
    /* Failed to create directories for file */
    RRC_UPDATE_EMKDIR,
    /* Failed to open/create output file for the extracted file. */
    RRC_UPDATE_EOPEN_OUTFILE,
    /* Failed to open or stat file in zip archive. */
    RRC_UPDATE_EOPEN_AR_FILE,
    /* Failed to read archive file contents. */
    RRC_UPDATE_EREAD_AR,
    /* Failed to write archive contents into output file on SD card. */
    RRC_UPDATE_EWRITE_OUT,
    /* Failed to write version.txt file. */
    RRC_UPDATE_EWRITE_VERSION,
    /* Failed to get free space on SD card */
    RRC_UPDATE_ESD_SZ,
    /* Not enough space on SD card to download ZIP */
    RRC_UPDATE_EZIP_SPC,
    /* Not enough space on SD card to extract ZIP */
    RRC_UPDATE_EZIP_EX_SPC,

    /* ZIP Errors */
    /* Failed to open the downloaded update ZIP file */
    RRC_UPDATE_EOPEN_ZIP
};

typedef union
{
    /* defined if ecode == ECURL */
    CURLcode ccode;
    /* defined for IO errors except INVFILE */
    int errnocode;
    /* defined for ZIP errors */
    int ziperr;
} rrc_update_result_inner;

struct rrc_update_result
{
    enum rrc_update_ecode ecode;
    rrc_update_result_inner inner;
};

/*
    Get the total size of all update ZIPs in bytes. This can be used to determine whether
    to warn the user that updating will take a long time based on some arbitrary threshold.

    On success, returns 0 and `size' is populated with the total download size. On failure,
    the return code is negative (usually a cURL error code), and `size' is zero.
*/
int rrc_update_get_total_update_size(struct rrc_update_state *state, curl_off_t *size);

/*
    Determines if a update or sequence of updates is large. This is determined based on
    if the total update download size is above some arbitrary value defined in
    RRC_UPDATE_LARGE_THRESHOLD, in bytes.

    Returns 0 if the update is not large (or there are none), 1 if it is large, and
    a negative error status on failure.
*/
int rrc_update_is_large(struct rrc_update_state *state, curl_off_t *size);

/*
    Does all updates specified in update_urls, in order.
    This involves sequentially donloading, unzipping, and applying each one
    TODO: maybe make this threaded so if we have multiple updates we can download
    one and apply one at the same time?

    Returns 0 on success and a negative code on fail.
    `res' is a pointer to a valid `struct rrc_update_result' on return.
*/
struct rrc_result rrc_update_do_updates_with_state(struct rrc_update_state *state);

/*
    Checks if updates are needed, and if there are, prompt the user and and download them. See `rrc_update_do_updates_with_state` for more details.
    This also writes the number of available updates into `count' and returns whether the updates were actually installed.
*/
struct rrc_result rrc_update_do_updates(void *xfb, int *count, bool *any_updates_installed);

#endif

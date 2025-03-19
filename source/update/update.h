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
};

/*
    Returns an int specifying version information from version.txt.
    E.g., 4.2.0 = 420
    Returns negative status on failure.
    SD driver must be loaded for this to work.
*/
int rrc_update_get_current_version();

/*
    Downloads a Retro Rewind ZIP. Uses the console to display progress.
*/
int rrc_update_download_zip(char *url, char **output);

/*
    Check for updates. If there are updates, the return code is 0 and `ret' is populated with
    URL information and amount of updates.
    If there are no updates, `ret' will be NULL and the return code will be 0.
    On failure, `ret' will be NULL and the return code will be negative.
*/
int rrc_update_check_for_updates(struct rrc_update_state *ret);

enum rrc_update_ecode
{
    /* Success */
    RRC_UPDATE_EOK = 0,
    /* CURL error. `ccode' is set to that error if this is set. */
    RRC_UPDATE_ECURL,
};

struct rrc_update_result
{
    /* always defined */
    enum rrc_update_ecode ecode;
    /* -1 if error is not a curl error */
    CURLcode ccode;
};

/*
    Convert an error failure code to a string for display.
*/
int rrc_update_ecode_to_string(int code);

/*
    Does all updates specified in update_urls, in order.
    This involves sequentially donloading, unzipping, and applying each one
    TODO: maybe make this threaded so if we have multiple updates we can download
    one and apply one at the same time?

    Returns 0 on success and a negative code on fail.
    `res' is a pointer to a valid `struct rrc_update_result' on return.
*/
int rrc_update_do_updates(struct rrc_update_state *state, struct rrc_update_result *res);

#endif
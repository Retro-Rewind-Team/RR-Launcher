/*
    result.h - Error and success handling implementation

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

#include <stdlib.h>
#include <unistd.h>
#include <gccore.h>

#include "prompt.h"
#include "result.h"

#include "console.h"

const struct rrc_result rrc_result_success = {
    .errtype = ESOURCE_NONE,
};

struct rrc_result rrc_result_create_error_curl(CURLcode error, const char *context)
{
    struct rrc_result res;

    res.errtype = ESOURCE_CURL;
    res.inner.ccode = error;
    res.context = context;

    return res;
}

struct rrc_result rrc_result_create_error_errno(int eno, const char *context)
{
    struct rrc_result res;

    res.errtype = ESOURCE_ERRNO;
    res.inner.errnocode = eno;
    res.context = context;

    return res;
}

struct rrc_result rrc_result_create_error_zip(int error, const char *context)
{
    struct rrc_result res;

    res.errtype = ESOURCE_ZIP;
    res.inner.ziperr = error;
    res.context = context;

    return res;
}

struct rrc_result rrc_result_create_error_corrupted_settingsfile(const char *context)
{
    struct rrc_result res;

    res.errtype = ESOURCE_CORRUPTED_SETTINGSFILE;
    res.context = context;

    return res;
}

struct rrc_result rrc_result_create_error_corrupted_versionfile(const char *context)
{
    struct rrc_result res;

    res.errtype = ESOURCE_CORRUPTED_VERSIONFILE;
    res.context = context;

    return res;
}

struct rrc_result rrc_result_create_error_misc_update(const char *context)
{
    struct rrc_result res;

    res.errtype = ESOURCE_UPDATE_MISC;
    res.context = context;

    return res;
}

bool rrc_result_is_error(struct rrc_result *result)
{
    return result != NULL && result->errtype != ESOURCE_NONE;
}

char *rrc_result_strerror(struct rrc_result *result)
{
    if (!rrc_result_is_error(result))
    {
        return NULL;
    }

    switch (result->errtype)
    {
    case ESOURCE_CURL:
        return (char *)curl_easy_strerror(result->inner.ccode);
    case ESOURCE_ERRNO:
        return strerror(result->inner.errnocode);
    case ESOURCE_ZIP:
        return "ZIP file error.";
    case ESOURCE_CORRUPTED_SETTINGSFILE:
        return "Corrupted settings file detected.";
    case ESOURCE_CORRUPTED_VERSIONFILE:
        return "Corrupted version file detected.";
    case ESOURCE_UPDATE_MISC:
        return "Update failed.";
    case ESOURCE_SD_CARD:
        return "SD card error.";
    case ESOURCE_WIISOCKET_INIT:
    {
        switch (result->inner.wiisocket_init_code)
        {
        case -1:
            return "Network initialisation already in progress.";
        case -2:
            return "Failed to initialise library.";
        case -3:
            return "Failed to initialise network.";
        case -4:
            return "Failed to register hardware in devoptab.";
        default:
            return "Unknown error code.";
        }
    }
    case ESOURCE_CORRUPTED_RR_XML:
        return "Invalid or corrupted RetroRewind6.xml.";
    default:
        return NULL;
    }
}

void rrc_result_error_check_error_normal(struct rrc_result *result, void *xfb)
{
    if (!rrc_result_is_error(result))
    {
        return;
    }

    int rows, cols;
    CON_GetMetrics(&cols, &rows);
    char line1[cols];
    snprintf(line1, cols, "Error: %s", rrc_result_strerror(result));

    char *lines[] = {
        line1,
        "",
        "Additional info:",
        (char *)result->context,
    };

    rrc_prompt_1_option(xfb, lines, 4, "OK");
}

void rrc_result_error_check_error_fatal(struct rrc_result *result)
{
    if (!rrc_result_is_error(result))
    {
        return;
    }

    const int origin_row = 0;

    rrc_con_clear(false);

    rrc_con_print_text_centered(origin_row, RRC_CON_ANSI_FG_BRIGHT_RED "A fatal error has occurred!");
    rrc_con_cursor_seek_to(origin_row + 2, 0);
    printf(RRC_CON_ANSI_FG_BRIGHT_RED "Error: " RRC_CON_ANSI_FG_WHITE "%s\n", rrc_result_strerror(result));
    printf(RRC_CON_ANSI_FG_BRIGHT_CYAN "Additional info: " RRC_CON_ANSI_FG_WHITE "%s\n", result->context);
    printf("\n\nPlease check your installation of Retro Rewind.\nThe launcher will exit in %i seconds.", RRC_RESULT_FATAL_SPLASH_TIME_SEC);

    usleep(RRC_RESULT_FATAL_SPLASH_TIME_SEC * 1000 * 1000);
    exit(1);
}

/*
    update.h - distribution update implementation
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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <gctypes.h>

#include "versionsfile.h"
#include "update.h"
#include "../util.h"
#include "../console.h"

#define _RRC_VERSIONFILE "RetroRewind6/version.txt"
#define _RRC_UPDATE_ZIP_NAME "update.zip"

int rrc_update_get_current_version()
{
    FILE *file = fopen(_RRC_VERSIONFILE, "r");
    if (file == NULL)
        return -5;

    int fd = fileno(file);
    struct stat statbuf;
    int res = stat(_RRC_VERSIONFILE, &statbuf);
    if (res != 0)
        return -4;

    int sz = statbuf.st_size;
    char verstring[sz + 1];
    if (read(fd, (void *)verstring, sz) == 0)
        return -3;

    if (fclose(file) != 0)
        return -2;

    /* add null termination */
    verstring[sz] = '\0';

    return rrc_versionsfile_parse_verstring(verstring);
}

int lp = -1;

int _rrc_zipdl_progress_callback(int *numinfo,
                                 curl_off_t dltotal,
                                 curl_off_t dlnow,
                                 curl_off_t ultotal,
                                 curl_off_t ulnow)
{
    int progress = (dlnow * 100) / dltotal;
    if (progress != lp)
    {
        lp = progress;
        char msg[100];
        snprintf(msg, 100, "Downloading update %i of %i (%i/%i kB)", ((*numinfo) / 100) + 1, (*numinfo) % 100, (int)(dlnow / (curl_off_t)1000), (int)(dltotal / (curl_off_t)1000));
        rrc_con_update(msg, progress);
    }
    return 0;
}

size_t _rrc_zipdl_write_data_callback(char *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int rrc_update_download_zip(char *url, char *filename, int current_zip, int max_zips)
{
    CURLcode cres;
    CURL *curl = curl_easy_init();
    FILE *fp;
    int numinfo = (current_zip * 100) + max_zips;
    if (curl)
    {
        fp = fopen(filename, "wb");
        if (fp == NULL)
        {
            return -100;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &numinfo);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, _rrc_zipdl_progress_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _rrc_zipdl_write_data_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        /* Perform the request, cres gets the return code */
        cres = curl_easy_perform(curl);

        /* Check for errors */
        if (cres != CURLE_OK)
        {
            // TODO: report error better
            printf("curl_easy_perform() failed: %s\n",
                   curl_easy_strerror(cres));

            fclose(fp);
            curl_easy_cleanup(curl);
            return -cres;
        }

        fclose(fp);
        curl_easy_cleanup(curl);
    }

    return 0;
}

int rrc_update_check_for_updates(struct rrc_update_state *ret)
{
    int current_ver = rrc_update_get_current_version();
    if (current_ver < 0)
    {
        ret = NULL;
        return current_ver;
    }

    return 0;
}

int rrc_update_do_updates(struct rrc_update_state *state, struct rrc_update_result *res)
{
    res->ecode = RRC_UPDATE_EOK;
    res->ccode = -1;

    while (state->current_update_num < state->num_updates)
    {
        char *url = state->update_urls[state->current_update_num];
        char *zip;
        int zipsize;
        int dlres = rrc_update_download_zip(url, _RRC_UPDATE_ZIP_NAME, state->current_update_num, state->num_updates);

        if (dlres == -100)
        {
            res->ccode = -1;
            res->ecode = RRC_UPDATE_INVFILE;
            return -1;
        }
        else if (dlres < 0)
        {
            res->ccode = -dlres;
            res->ecode = RRC_UPDATE_ECURL;
            return -1;
        }

        struct stat sb;
        int s = stat(_RRC_UPDATE_ZIP_NAME, &sb);
        if (s == -1)
        {
            res->ccode = -1;
            res->ecode = RRC_UPDATE_INVFILE;
            return -1;
        }

        rrc_dbg_printf("update size: %llu bytes\n", sb.st_size);

        /* TODO: apply zip (probably in a new method rrc_update_apply_zip or something)
           Ideally would have proper progress reporting for how much has been applied but
           not sure how feasible this is?
           Need to stat the amount of files and stuff for that maybe?
        */
        rrc_con_update("Pretend it's applying the zip lol", 50);

        usleep(5000000);

        int rres = remove(_RRC_UPDATE_ZIP_NAME);
        if (rres == -1)
        {
            res->ccode = -1;
            res->ecode = RRC_UPDATE_INVFILE;
            return -1;
        }

        state->current_update_num++;
    }

    return 0;
}
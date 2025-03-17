/*
    versionsfile.c - fetching and parsing of version info implementation
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
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <string.h>

#include "../console.h"
#include "versionsfile.h"

#define _RRC_VERSIONSFILE_URL "http://update.rwfc.net:8000/RetroRewind/RetroRewindVersion.txt"

struct versioninfo
{
    int len;
    char *ptr;
};

int _rrc_versionsfile_progress_callback(void *unused,
                                        curl_off_t dltotal,
                                        curl_off_t dlnow,
                                        curl_off_t ultotal,
                                        curl_off_t ulnow)
{
    static int lp = -1;
    int progress = (dlnow * 100) / dltotal;
    if (progress != lp)
    {
        lp = progress;
        rrc_con_update("Fetching Version Info", progress);
    }
    return 0;
}

size_t _rrc_versionsfile_write_callback(char *ptr, size_t size, size_t nmemb, void *ss)
{
    struct versioninfo *s = (struct versioninfo *)ss;
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL)
    {
        // TODO: maybe make a proper place to put error info?
        printf("realloc failed!");
        return -1;
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

int rrc_versionsfile_get_versions(char **result)
{
    struct versioninfo s;
    s.len = 0;
    s.ptr = malloc(s.len + 1);
    if (s.ptr == NULL)
    {
        printf("malloc failed!");
        return -1;
    }
    s.ptr[0] = '\0';

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, _RRC_VERSIONSFILE_URL);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, _rrc_versionsfile_progress_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _rrc_versionsfile_write_callback);
        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return 0;
}
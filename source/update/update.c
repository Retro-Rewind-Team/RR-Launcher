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
#include <curl/curl.h>

#include "versionsfile.h"
#include "update.h"
#include "../console.h"

#define _RRC_VERSIONFILE "RetroRewind6/version.txt"

/*
    Returns an int specifying version information from a verstring.
    E.g., 4.2.0 = 420
    Returns -1 on failure.
*/
int _rrc_parse_verstring(char *verstring)
{
    /* major, minor, revision */
    int parts[3] = {0, 0, 0};
    int current = 0, started_at = 0;
    int len = strlen(verstring);
    for (int i = 0; i < len + 1; i++)
    {
        /* read until . or EOF, then extract that section */
        if (verstring[i] == '.' || verstring[i] == '\0')
        {
            int sect_len = i - started_at;
            if (sect_len == 0)
                /* ??? */
                return -1;

            /* read this section */
            char section[sect_len + 1];
            int idx = 0;
            for (int j = started_at; j < started_at + sect_len; j++)
            {
                section[idx] = verstring[j];
                idx++;
            }
            section[sect_len] = '\0';

            int section_l = strtol(section, NULL, 10);
            parts[current] = section_l;
            current++;
            if (current > 2)
            {
                /* we read the rev now */
                break;
            }

            started_at = i + 1;
        }
    }

    int final = 0;
    final += (parts[0] * 100);
    final += (parts[1] * 10);
    final += parts[2];

    return final;
}

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

    return _rrc_parse_verstring(verstring);
}

int _rrc_zipdl_progress_callback(char *url,
                                 curl_off_t dltotal,
                                 curl_off_t dlnow,
                                 curl_off_t ultotal,
                                 curl_off_t ulnow)
{
    static int lp = -1;
    int url_len = strlen(url);
    char msg[url_len + 15];
    snprintf(msg, url_len + 15, "Downloading %s", url);
    int progress = (dlnow * 100) / dltotal;
    if (progress != lp)
    {
        lp = progress;
        rrc_con_update(msg, progress);
    }
    return 0;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    return size * nmemb;
}

int rrc_update_download_zip(char *url)
{
    CURLcode cres;
    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, url);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, _rrc_zipdl_progress_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        /* Perform the request, cres gets the return code */
        cres = curl_easy_perform(curl);
        /* Check for errors */
        if (cres != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(cres));

        printf("status code: %d", cres);

        /* always cleanup */
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
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
// max array size
#define _RRC_SPLIT_LIM 4096

int rrc_versionsfile_parse_verstring(char *verstring)
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

int rrc_versionsfile_get_versionsfile(char **result)
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

        if (res != CURLE_OK)
        {
            // TODO: report error better
            printf("curl_easy_perform() failed: %s\n",
                   curl_easy_strerror(res));
            return -res;
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    if (result != NULL)
    {
        *result = s.ptr;
    }

    return 0;
}

int rrc_versionsfile_split_by(char *in, char by, char ***out, int *amt)
{
    int len = strlen(in), i = 0, count = 0, current_len = 0, start_idx = 0;
    *out = malloc(_RRC_SPLIT_LIM);
    if (*out == NULL)
    {
        return -2;
    }

    for (; i < len; i++)
    {
        if (in[i] == by || i == (len - 1))
        {
            /* handle last element */
            if (i == (len - 1) && in[i] != by)
            {
                current_len++;
            }
            /* +1 for NULL termination */
            char *this = malloc(current_len + 1);
            if (this == NULL)
            {
                rrc_versionsfile_free_split(*out, count);
                return -1;
            }

            /* copy from offset to new array */
            memcpy(this, in + start_idx, current_len);
            this[current_len] = '\0';
            (*out)[count] = this;

            start_idx = i + 1;
            count++;
            if (amt != NULL)
                *amt = count;
            if (count > _RRC_SPLIT_LIM)
            {
                return 2;
            }

            current_len = 0;
        }
        else
        {
            current_len++;
        }
    }

    if (amt != NULL)
        *amt = count;
    return 0;
}

void rrc_versionsfile_free_split(char **array, int count)
{
    for (int i = 0; i < count; i++)
    {
        free(array[i]);
    }
    free(array);
}

int rrc_versionsfile_get_necessary_urls(char *versionsfile, int current_version, int *uamt, char ***result)
{
    /*
        We need to read the file line-wise and also space-wise.
        The format of the file is each line has one entry. An entry is of the form:

        version url

        Where version is a normal verstring we can parse to an int, and url is the zip
        url for that version.
        We parse each verstring, and if it yields a greater absolute value than our current version,
        we parse the url associated with it and ass it to the list of updates.
    */
    *result = malloc(_RRC_SPLIT_LIM);

    char **lines;
    int count;
    int res = rrc_versionsfile_split_by(versionsfile, '\n', &lines, &count);
    if (res < 0)
    {
        // TODO: report these errors better
        printf("failed to split versionfile: ret = %i\n", res);
        return res;
    }
    else if (res == 2)
    {
        printf("versionfile had greater than 4096 entries!!! corrupted file?\n");
        rrc_versionsfile_free_split(lines, count);
        return -1;
    }

    int update_idx = 0;

    /* This is where we go through each line until we find the first one we care about and set it to first_update_idx */
    for (int i = 0; i < count; i++)
    {
        char *line = lines[i];
        char **parts;
        /* this is only here if we fail to parse it all and need to free this amount */
        int num_parts;
        int res = rrc_versionsfile_split_by(line, ' ', &parts, &num_parts);
        /* dont bother checking for 2 because it really should never ever happen */

        if (res < 0)
        {
            // TODO: report these errors better
            printf("failed to split versionfile: ret = %i\n", res);
            rrc_versionsfile_free_split(lines, count);
            return res;
        }

        int verint = rrc_versionsfile_parse_verstring(parts[0]);

        if (verint == -1)
        {
            rrc_versionsfile_free_split(lines, count);
            /* give this unique error code */
            return -3;
        }

        if (verint > current_version)
        {
            (*result)[update_idx] = parts[1];
            update_idx++;
        }
        else
        {
            free(parts[1]);
        }

        free(parts[0]);
        free(parts);
    }

    if (update_idx == 0) /* no updates needed */
    {
        *result = NULL;
    }

    *uamt = update_idx;

    rrc_versionsfile_free_split(lines, count);
    return 0;
}
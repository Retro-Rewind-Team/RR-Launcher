/*
    versionsfile.h - fetching and parsing of version info header
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

#ifndef RRC_VERSIONSFILE_H
#define RRC_VERSIONSFILE_H

/*
    Get version information from Retro Rewind servers.
    On success, return code is 0 and `result' is populated with a NULL-terminated string.
    On failure, return code is negative CURL return code and `result' is NULL.
*/
int rrc_versionsfile_get_versions(char **result);

#endif
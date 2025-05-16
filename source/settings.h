/*
    settings.h - header for the settings menu

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

#ifndef RRC_SETTINGS_H
#define RRC_SETTINGS_H

#include "settingsfile.h"
#include "result.h"

enum rrc_settings_result
{
    RRC_SETTINGS_ERROR = -1,
    RRC_SETTINGS_LAUNCH = 0,
    RRC_SETTINGS_EXIT = 1
};

// TODO: move xfb to some kind of global descriptor
/*
    Displays settings and returns the selected option to perform after closing.

    Note that if `result' is an error type, the return value is RRC_SETTINGS_ERROR.
*/
enum rrc_settings_result rrc_settings_display(void *xfb, struct rrc_settingsfile *stored_settings, struct rrc_result *result);

#endif

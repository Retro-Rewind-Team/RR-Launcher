/*
    time.h - Time-related functions.

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

#ifndef RRC_TIME_H
#define RRC_TIME_H

#include <gctypes.h>

typedef s64 rrc_time_tick;

/**
 * Returns the difference of two ticks in milliseconds.
 */
u32 diff_msec(rrc_time_tick start, rrc_time_tick end);

/**
 * Gets the time in ticks.
 * The return value is usually only meaningful when comparing it to another tick, e.g. using one of the `diff_*` functions.
 */
rrc_time_tick gettime();

/**
 * Sleeps for a given amount of microseconds.
 * This is a wrapper around usleep() that also periodically checks for shutdown requests.
 */
void rrc_usleep(u32 usec);

#endif

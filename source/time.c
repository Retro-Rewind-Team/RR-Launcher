/*
    time.c - Time-related function implementations.

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

#include <types.h>
#include <unistd.h>
#include "time.h"
#include "shutdown.h"

void rrc_usleep(u32 usec)
{
#define LONG_USLEEP (10 * 1000 * 1000) // 10 seconds
#define LONG_UPERIOD (100 * 1000)      // 100ms
#define SHORT_UPERIOD (1000)           // 1ms

    // For very long sleeps, use a slightly higher period to reduce the number of wakeups
    u32 period = usec > LONG_USLEEP ? LONG_UPERIOD : SHORT_UPERIOD;

    for (int i = 0; i < usec / period; i++)
    {
        usleep(period);
        rrc_shutdown_check();
    }

    u32 rem = usec % period;
    if (rem > 0)
    {
        usleep(rem);
        rrc_shutdown_check();
    }
}

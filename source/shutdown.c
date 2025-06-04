/*
    shutdown.c - Shutdown and reset handler checker

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

#include <gccore.h>

#include "console.h"
#include "time.h"

#define RRC_SHUTDOWN_CHECK_TIME_MS 2500

static volatile int _shutdown_state = -1;
/* Only actually trigger the shutdown if we check for it within RRC_SHUTDOWN_CHECK_TIME_MS -
   This is handy in the event we get this during an update and may not check until a minute
   or two later */
static rrc_time_tick _shutdown_time = 0;

void power_callback()
{
    _shutdown_time = gettime();
    _shutdown_state = SYS_POWEROFF;
}

void reset_callback(u32 irq, void *ctx)
{
    _shutdown_time = gettime();
    _shutdown_state = SYS_RETURNTOMENU;
}

void rrc_shutdown_register_callbacks()
{
    SYS_SetPowerCallback(power_callback);
    SYS_SetResetCallback(reset_callback);
}

void rrc_shutdown_check()
{
    int time_since_trigger = diff_msec(_shutdown_time, gettime());

    if (_shutdown_state != -1 && time_since_trigger < RRC_SHUTDOWN_CHECK_TIME_MS)
    {
        rrc_con_update("Shutting Down", 100);
        SYS_ResetSystem(_shutdown_state, 0, 0);
    }
    /* set state back to off if not triggered in time */
    else if (time_since_trigger >= RRC_SHUTDOWN_CHECK_TIME_MS)
    {
        _shutdown_state = -1;
    }
}
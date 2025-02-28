/*
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

#include <ogc/lwp.h>
#include <ogc/video.h>
#include <wiiuse/wpad.h>
#include "util.h"
#include "shutdown.h"

bool rrc_shutting_down = false;

#define SHUTDOWN_LOOP_DELAY 10000 /* 10ms */

static void *rrc_shutdown_handler(void *)
{
    while (1)
    {
        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
        {
            rrc_shutting_down = true;
            break;
        }

        usleep(SHUTDOWN_LOOP_DELAY);
        VIDEO_WaitVSync();
    }

    rrc_dbg_printf("end of shutdown handler");
    return NULL;
}

lwp_t rrc_shutdown_spawn()
{
    lwp_t thread;
    RRC_ASSERTEQ(
        LWP_CreateThread(&thread, rrc_shutdown_handler, NULL, NULL, 0, RRC_LWP_PRIO_IDLE), RRC_LWP_OK, "LWP_CreateThread for shutdown handler");
    return thread;
}

void rrc_shutdown_join(lwp_t thread)
{
    RRC_ASSERTEQ(LWP_JoinThread(thread, NULL), 0, "LWP_JoinThread for shutdown handler");
}

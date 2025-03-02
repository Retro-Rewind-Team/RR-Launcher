/*
    shutdown.h - Interface for the shutdown background thread, waits for Home
    wpad presses and notifies other threads that periodically call `CHECK_EXIT()`.

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

#ifndef RRC_SHUTDOWN_H
#define RRC_SHUTDOWN_H

#include "res.h"

#define CHECK_EXIT()                                       \
    if (rrc_shutting_down)                                 \
    {                                                      \
        rrc_dbg_printf("Home button pressed, exiting..."); \
        return RRC_RES_SHUTDOWN_INTERRUPT;                 \
    }

extern bool rrc_shutting_down;

/**
 * Spawns the background thread for listening to Home menu button presses.
 */
lwp_t rrc_shutdown_spawn();

/**
 * Waits until the home button is pressed and the background thread exits.
 */
void rrc_shutdown_join(lwp_t);

#endif

/*
    exception.h - exception stub handler definition

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

#ifndef RRC_EXCEPTION_H
#define RRC_EXCEPTION_H

#include <gccore.h>

/* amount of time to display exception screen before exiting */
#define EXCEPTION_DISPLAY_TIME_SEC 10

void init_exception_handlers();

/* in the event this is ever useful, provide it */
void deinit_exception_handlers();

#endif
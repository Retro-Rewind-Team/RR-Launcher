/*
    res.h - return codes from rrc routines

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

#ifndef RRC_RES_H
#define RRC_RES_H

enum rrc_res
{
    /*
     * Success status
     */
    RRC_RES_OK = 0,
    /*
     * Status for a shutdown interrupt
     */
    RRC_RES_SHUTDOWN_INTERRUPT = 3000
};

#endif
/*
	di.h - /dev/di implementation header

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

#include <gctypes.h>

#ifndef RRC_DI_H
#define RRC_DI_H

#define RRC_DI_LIBDI_OK 0

/* used for raw ioctls */
enum rrc_di_ret
{
    RRC_DI_RET_UNKNOWN = 0,
    RRC_DI_RET_OK = 1,
};

struct rrc_di_diskid
{
    char game_id[4];
    u16 group_id;
    u8 disc_num;
    u8 disc_ver;
};

int rrc_di_getfd();

int rrc_di_init();

int rrc_di_reset();

int rrc_di_getlowcoverregister(unsigned int* status);

int rrc_di_getdiskid(struct rrc_di_diskid* diskid);

#endif
/*
	di.c - /dev/di wrapper implementation

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

/* /dev/di is documented at https://wiibrew.org/wiki//dev/di */

/*
    This file exists because some aspects of libdi seem to be implemented incorrectly.
    Speficially, `init' opens the device file with the wrong mode (2 instead of 0), and
    attempting to get cover register status uses the wrong command number. We provide
    our own implementations where necessary, but all manipulation of /dev/di should
    regardless happen through this file due to the fact that it all shares the same 
    global di_fd variable.
*/
#include <di/di.h>

#include "string.h"
#include "di.h"
#include "util.h"

int rrc_di_getfd()
{
    // defined in di/di.h
    return di_fd;
}

int rrc_di_init()
{
	int fd = (int)IOS_Open("/dev/di", 0);

	di_fd = fd;
    return fd;
}

int rrc_di_reset()
{
    return DI_Reset();
}

int rrc_di_getlowcoverregister(unsigned int* status)
{
	uint32_t outbuf[8] __attribute__((aligned(32))) = {0};
	uint32_t dic[8] __attribute__((aligned(32)));
	memset(dic, 0x00, 0x20);
	dic[0] = 0x7A << 24;

	// get drive status (disc inserted?)
	int res = IOS_Ioctl(rrc_di_getfd(), 0x7A, dic, 0x20, outbuf, 0x20);
	*status = outbuf[0];

    return res;
}

int rrc_di_getdiskid(struct rrc_di_diskid* diskid)
{
    uint64_t idr;
	int resdid = DI_ReadDiscID(&idr);

    if(resdid == RRC_DI_LIBDI_OK)
    {
        memcpy(diskid, &idr, sizeof(struct rrc_di_diskid));
    } else {
        // zero it on error
        memset(diskid, 0, sizeof(struct rrc_di_diskid));
    }

    return resdid;
}
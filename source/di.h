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

#define RRC_DI_PART_TYPE_DATA 0

#define RRC_DI_LIBDI_OK 0

#define RRC_DI_PART_GROUPS_OFFSET 0x40000
#define RRC_DI_DATA_PART_HEADER 0x420

/* used for raw ioctls */
enum rrc_di_ret
{
    RRC_DI_RET_UNKNOWN = 0,
    RRC_DI_RET_OK = 1,
};

struct rrc_di_disk_id
{
    char game_id[4];
    u16 group_id;
    u8 disc_num;
    u8 disc_ver;
};

/**
 * An entry in the partition group table located at 0x40000 on the disk.
 * See: https://wiibrew.org/wiki/Wii_disc#Partitions_information
 */
struct rrc_di_part_group
{
    u32 count;
    u32 offset;
};

/**
 * Partition information, located at any given `rrc_part_entry.offset` (`count` times).
 * See: https://wiibrew.org/wiki/Wii_disc#Partition_table_entry
 */
struct rrc_di_part_info
{
    u32 offset;
    u32 type;
};

/**
 * The header of the data partition, located at offset 0x420.
 * Encrypted and needs to be read using DI_Read() rather than DI_UnencryptedRead().
 */
struct rrc_di_data_part_header
{
    u32 dol_offset;
    u32 fst_offset;
    u32 fst_size;
};

int rrc_di_getfd();

int rrc_di_init();

int rrc_di_reset();

int rrc_di_get_low_cover_register(unsigned int *status);

int rrc_di_get_disk_id(struct rrc_di_disk_id *diskid);

int rrc_di_unencrypted_read(void *buf, u32 size, u32 offset);

int rrc_di_read(void *buf, u32 size, u32 offset);

int rrc_di_open_partition(u32 offset);

#endif

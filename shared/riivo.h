/*
    riivo.h - definition Riivolution types for XML parsing

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

#ifndef RRC_RIIVO_H
#define RRC_RIIVO_H

#include "types.h"

#define RRC_RIIVO_XML_PATH "RetroRewind6/xml/RetroRewind6.xml"

enum rrc_riivo_disc_replacement_type
{
    RRC_RIIVO_FILE_REPLACEMENT,
    RRC_RIIVO_FOLDER_REPLACEMENT,
};

struct rrc_riivo_disc_replacement
{
    enum rrc_riivo_disc_replacement_type type;
    const char *external;
    const char *disc;
};

struct rrc_riivo_disc
{
    u32 count;
    struct rrc_riivo_disc_replacement replacements[0];
};

struct rrc_riivo_memory_patch
{
    u32 addr;
    u32 value;
    u32 original; // uninitialized if !original_init
    bool original_init;
};

#endif

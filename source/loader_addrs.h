/*
    loader_addrs.h - DVD patch addresses per region

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

/*
    These backjmp arrays are created by objdumping C code.

    The offsets are found using versions.txt on the main Pulsar repository.
*/
#ifndef RRC_LOADER_BACKJMP_H
#define RRC_LOADER_BACKJMP_H

#include <gctypes.h>

#include "result.h"

enum rrc_dvd_region
{
    // PAL
    RRC_DVD_REGION_P = 0,
    // NTSC America
    RRC_DVD_REGION_E = 1,
    // NTSC Japan
    RRC_DVD_REGION_J = 2
};

enum rrc_dvd_function
{
    RRC_DVDF_CONVERT_PATH_TO_ENTRYNUM = 0,
    RRC_DVDF_FAST_OPEN = 1,
    RRC_DVDF_OPEN = 2,
    RRC_DVDF_READ_PRIO = 3,
    RRC_DVDF_CLOSE = 4
};

// This is queried to get the correct DVD function addresses for the region.
// TODO: NTSC-K support?
const u32 rrc_dvdf_addrs[3][5] =
    {
        // 80000000-*: +0x0
        [RRC_DVD_REGION_P] =
            {
                [RRC_DVDF_CONVERT_PATH_TO_ENTRYNUM] = 0x8015df4c,
                [RRC_DVDF_FAST_OPEN] = 0x8015e254,
                [RRC_DVDF_OPEN] = 0x8015e2bc,
                [RRC_DVDF_READ_PRIO] = 0x8015e834,
                [RRC_DVDF_CLOSE] = 0x8015e568},
        // 8000af24-8000b6b3: -0xa0
        [RRC_DVD_REGION_E] =
            {
                [RRC_DVDF_CONVERT_PATH_TO_ENTRYNUM] = 0x8015deac,
                [RRC_DVDF_FAST_OPEN] = 0x8015e1b4,
                [RRC_DVDF_OPEN] = 0x8015e21c,
                [RRC_DVDF_READ_PRIO] = 0x8015e794,
                [RRC_DVDF_CLOSE] = 0x8015e4c8},
        // 80021bac-80244ddf: -0xe0
        [RRC_DVD_REGION_J] =
            {
                [RRC_DVDF_CONVERT_PATH_TO_ENTRYNUM] = 0x8015de6c,
                [RRC_DVDF_FAST_OPEN] = 0x8015e174,
                [RRC_DVDF_OPEN] = 0x8015e1dc,
                [RRC_DVDF_READ_PRIO] = 0x8015e754,
                [RRC_DVDF_CLOSE] = 0x8015e488}};

// These instructions store the address of the original DVD function in a specific register
// and then jump to it. The only difference in each set is the address being jumped to (i.e., the second instruction)
// We include all 4 for every case for completeness and extensibility, if ever needed.
const u32 rrc_dvdf_backjmp_instrs[3][5][4] = {
    [RRC_DVD_REGION_P] =
        {
            [RRC_DVDF_CONVERT_PATH_TO_ENTRYNUM] = {0x3d208015, 0x6129df5c, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_FAST_OPEN] = {0x3d208015, 0x6129e264, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_OPEN] = {0x3d208015, 0x6129e2cc, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_READ_PRIO] = {0x3d208015, 0x6129e844, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_CLOSE] = {0x3d208015, 0x6129e578, 0x7d2903a6, 0x4e800420}},
    [RRC_DVD_REGION_E] =
        {
            [RRC_DVDF_CONVERT_PATH_TO_ENTRYNUM] = {0x3d208015, 0x6129debc, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_FAST_OPEN] = {0x3d208015, 0x6129e1c4, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_OPEN] = {0x3d208015, 0x6129e22c, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_READ_PRIO] = {0x3d208015, 0x6129e7a4, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_CLOSE] = {0x3d208015, 0x6129e4d8, 0x7d2903a6, 0x4e800420}},
    [RRC_DVD_REGION_J] =
        {
            [RRC_DVDF_CONVERT_PATH_TO_ENTRYNUM] = {0x3d208015, 0x6129de7c, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_FAST_OPEN] = {0x3d208015, 0x6129e184, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_OPEN] = {0x3d208015, 0x6129e1ec, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_READ_PRIO] = {0x3d208015, 0x6129e764, 0x7d2903a6, 0x4e800420},
            [RRC_DVDF_CLOSE] = {0x3d208015, 0x6129e498, 0x7d2903a6, 0x4e800420}}};

// We need to be able to jump to the custom functions. 
// These jump to the approprate address for each custom function.
const u32 rrc_dvdf_jmp_to_custom_instrs[5][4] = {
    [RRC_DVDF_CONVERT_PATH_TO_ENTRYNUM] = {0x3d208178, 0x61292e60, 0x7d2903a6, 0x4e800420},
    [RRC_DVDF_FAST_OPEN] = {0x3d208178, 0x61292ee0, 0x7d2903a6, 0x4e800420},
    [RRC_DVDF_OPEN] = {0x3d208178, 0x61292ea0, 0x7d2903a6, 0x4e800420},
    [RRC_DVDF_READ_PRIO] = {0x3d208178, 0x61292f20, 0x7d2903a6, 0x4e800420},
    [RRC_DVDF_CLOSE] = {0x3d208178, 0x61292f60, 0x7d2903a6, 0x4e800420}
};

enum rrc_dvd_region rrc_region_char_to_region(char region)
{
    switch (region)
    {
    case 'P':
        return RRC_DVD_REGION_P;
    case 'E':
        return RRC_DVD_REGION_E;
    case 'J':
        return RRC_DVD_REGION_J;
    default:
        return -1;
    }
}

#endif
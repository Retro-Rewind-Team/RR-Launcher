/*
    dvd.h - DVD replacement function- and helper type declarations

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

#ifndef RRC_RUNTIME_EXT_DVD
#define RRC_RUNTIME_EXT_DVD

#include <gctypes.h>

typedef struct FileInfo FileInfo;
typedef struct CommandBlock CommandBlock;

typedef void (*Callback)(s32 result, FileInfo *fileInfo);
typedef void (*CBCallback)(s32 result, CommandBlock *block);

typedef struct
{
    char gameName[4];
    char company[2];
    u8 diskNumber;
    u8 gameVersion;
    u8 streaming;
    u8 streamingBufSize;
    u8 padding[14];
    u32 rvlMagic;
    u32 gcMagic;
} DiskID;

struct CommandBlock
{
    CommandBlock *next;
    CommandBlock *prev;
    u32 command;
    u32 state;
    u32 offset;
    u32 size;
    void *buffer;
    u32 curTransferSize;
    u32 transferredSize;
    DiskID *id;
    CBCallback callback;
    void *userData;
};

struct FileInfo
{
    CommandBlock commandBlock;
    u32 startAddr;
    u32 length;
    Callback callback;
};

s32 custom_convert_path_to_entry_num_impl(const char *filename);
s32 custom_open_impl(const char *path, FileInfo *file_info);
s32 custom_fast_open_impl(s32 entry_num, FileInfo *file_info);
s32 custom_read_prio_impl(FileInfo *file_info, void *buffer, s32 length, s32 offset, s32 prio);
bool custom_close_impl(FileInfo *file_info);

#endif

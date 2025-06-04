/*
    linker-E.ld - Linker script for NTSC runtime-ext

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

SECTIONS {
    # .init, .text and .rodata are specified in Makefile LDFLAGS
    .dvd_convert_path_to_entrynum 0x81782e60 : { *(.dvd_convert_path_to_entrynum) }
    .dvd_open                     0x81782ea0 : { *(.dvd_open) }
    .dvd_fast_open                0x81782ee0 : { *(.dvd_fast_open) }
    .dvd_read_prio                0x81782f20 : { *(.dvd_read_prio) }
    .dvd_close                    0x81782f60 : { *(.dvd_close) }
    .riivo_disc_ptr               0x81782fa0 : { *(.riivo_disc_ptr) }

    # Functions provided by the game
    PROVIDE(__lcase = 0x80246008);
    PROVIDE(__ucase = 0x80246108);
    PROVIDE(OSUTF8to32 = 0x801ab370);
    PROVIDE(OSUTF32toANSI = 0x801ab4f0);
    PROVIDE(OSLockMutex = 0x801a7e44);
    PROVIDE(OSUnlockMutex = 0x801a7f20);
    PROVIDE(OSGetCurrentThread = 0x801a9810);
    PROVIDE(OSGetTime = 0x801aacbc);
    PROVIDE(OSTicksToCalendarTime = 0x801aaf08);
    PROVIDE(VIWaitForRetrace = 0x801b994c);
    PROVIDE(OS_Report = 0x801a2530);
    PROVIDE(OS_Fatal = 0x801a4e24);
    PROVIDE(msleep = 0x800f2470);
    PROVIDE(snprintf = 0x80010dd8);
    PROVIDE(iosCreateHeap = 0x80194918);
    PROVIDE(iosAlloc = 0x80194a48);
    PROVIDE(iosAllocAligned = 0x80194c4c);
    PROVIDE(iosFree = 0x80194c50);
    PROVIDE(IOS_OpenAsync = 0x80193740);
    PROVIDE(IOS_Open = 0x80193858);
    PROVIDE(IOS_CloseAsync = 0x80193978);
    PROVIDE(IOS_Close = 0x80193a38);
    PROVIDE(IOS_ReadAsync = 0x80193ae0);
    PROVIDE(IOS_Read = 0x80193be0);
    PROVIDE(IOS_WriteAsync = 0x80193ce8);
    PROVIDE(IOS_Write = 0x80193de8);
    PROVIDE(IOS_SeekAsync = 0x80193ef0);
    PROVIDE(IOS_Seek = 0x80193fd0);
    PROVIDE(IOS_IoctlAsync = 0x801940b8);
    PROVIDE(IOS_Ioctl = 0x801941f0);
    PROVIDE(__ios_Ioctlv = 0x80194320);
    PROVIDE(IOS_IoctlvAsync = 0x8019445c);
    PROVIDE(IOS_Ioctlv = 0x80194540);
    PROVIDE(IOS_IoctlvReboot = 0x8019461c);
    PROVIDE(DCFlushRange = 0x801a158c);
    PROVIDE(ICInvalidateRange = 0x801a1670);
    PROVIDE(DVDCancel = 0x80162f4c);
}
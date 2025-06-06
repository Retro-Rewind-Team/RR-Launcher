/*
    linker-E.ld - Linker script for NTSC-J runtime-ext

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

INCLUDE base.ld

SECTIONS {
    # Functions provided by the game
    PROVIDE(__lcase = 0x80245cd0);
    PROVIDE(__ucase = 0x80245dd0);
    PROVIDE(OSUTF8to32 = 0x801ab330);
    PROVIDE(OSUTF32toANSI = 0x801ab4b0);
    PROVIDE(OSLockMutex = 0x801a7e04);
    PROVIDE(OSUnlockMutex = 0x801a7ee0);
    PROVIDE(OSGetCurrentThread = 0x801a97d0);
    PROVIDE(OSGetTime = 0x801aac7c);
    PROVIDE(OSTicksToCalendarTime = 0x801aaec8);
    PROVIDE(VIWaitForRetrace = 0x801b990c);
    PROVIDE(OS_Report = 0x801a24f0);
    PROVIDE(OS_Fatal = 0x801a4de4);
    PROVIDE(msleep = 0x800f2430);
    PROVIDE(snprintf = 0x8001185c);
    PROVIDE(iosCreateHeap = 0x801948d8);
    PROVIDE(iosAlloc = 0x80194a08);
    PROVIDE(iosAllocAligned = 0x80194c0c);
    PROVIDE(iosFree = 0x80194c10);
    PROVIDE(IOS_OpenAsync = 0x80193700);
    PROVIDE(IOS_Open = 0x80193818);
    PROVIDE(IOS_CloseAsync = 0x80193938);
    PROVIDE(IOS_Close = 0x801939f8);
    PROVIDE(IOS_ReadAsync = 0x80193aa0);
    PROVIDE(IOS_Read = 0x80193ba0);
    PROVIDE(IOS_WriteAsync = 0x80193ca8);
    PROVIDE(IOS_Write = 0x80193da8);
    PROVIDE(IOS_SeekAsync = 0x80193eb0);
    PROVIDE(IOS_Seek = 0x80193f90);
    PROVIDE(IOS_IoctlAsync = 0x80194078);
    PROVIDE(IOS_Ioctl = 0x801941b0);
    PROVIDE(__ios_Ioctlv = 0x801942e0);
    PROVIDE(IOS_IoctlvAsync = 0x8019441c);
    PROVIDE(IOS_Ioctlv = 0x80194500);
    PROVIDE(IOS_IoctlvReboot = 0x801945dc);
    PROVIDE(DCFlushRange = 0x801a154c);
    PROVIDE(ICInvalidateRange = 0x801a1630);
    PROVIDE(DVDCancel = 0x80162f0c);
}

# Linker script for NTSC-J runtime-ext

SECTIONS {
    # .init, .text and .rodata are specified in Makefile LDFLAGS
    .dvd_convert_path_to_entrynum 0x81782e60 : { *(.dvd_convert_path_to_entrynum) }
    .dvd_open                     0x81782ea0 : { *(.dvd_open) }
    .dvd_fast_open                0x81782ee0 : { *(.dvd_fast_open) }
    .dvd_read_prio                0x81782f20 : { *(.dvd_read_prio) }
    .dvd_close                    0x81782f60 : { *(.dvd_close) }
    .riivo_disc_ptr               0x81782fa0 : { *(.riivo_disc_ptr) }

    # Functions provided by the game
    PROVIDE(__lcase = 0x80246320);
    PROVIDE(__ucase = 0x80246420);
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
}
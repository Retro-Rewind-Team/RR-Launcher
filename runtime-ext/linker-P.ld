# Linker script for PAL runtime-ext

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
    PROVIDE(OSUTF8to32 = 0x801ab410);
    PROVIDE(OSUTF32toANSI = 0x801ab590);
    PROVIDE(OSLockMutex = 0x801a7ee4);
    PROVIDE(OSUnlockMutex = 0x801a7fc0);
    PROVIDE(OSGetCurrentThread = 0x801a98b0);
    PROVIDE(OSGetTime = 0x801aad5c);
    PROVIDE(OSTicksToCalendarTime = 0x801aafa8);
    PROVIDE(VIWaitForRetrace = 0x801b99ec);
    PROVIDE(OS_Report = 0x801A25D0);
    PROVIDE(OS_Fatal = 0x801A4EC4);
    PROVIDE(msleep = 0x800f2510);
    PROVIDE(snprintf = 0x80011938);
    PROVIDE(iosCreateHeap = 0x801949b8);
    PROVIDE(iosAlloc = 0x80194ae8);
    PROVIDE(iosAllocAligned = 0x80194cec);
    PROVIDE(iosFree = 0x80194cf0);
    PROVIDE(IOS_OpenAsync = 0x801937e0);
    PROVIDE(IOS_Open = 0x801938f8);
    PROVIDE(IOS_CloseAsync = 0x80193a18);
    PROVIDE(IOS_Close = 0x80193ad8);
    PROVIDE(IOS_ReadAsync = 0x80193b80);
    PROVIDE(IOS_Read = 0x80193c80);
    PROVIDE(IOS_WriteAsync = 0x80193d88);
    PROVIDE(IOS_Write = 0x80193e88);
    PROVIDE(IOS_SeekAsync = 0x80193f90);
    PROVIDE(IOS_Seek = 0x80194070);
    PROVIDE(IOS_IoctlAsync = 0x80194158);
    PROVIDE(IOS_Ioctl = 0x80194290);
    PROVIDE(__ios_Ioctlv = 0x801943c0);
    PROVIDE(IOS_IoctlvAsync = 0x801944fc);
    PROVIDE(IOS_Ioctlv = 0x801945e0);
    PROVIDE(IOS_IoctlvReboot = 0x801946bc);
    PROVIDE(DCFlushRange = 0x801a162c);
    PROVIDE(ICInvalidateRange = 0x801a1710);
}

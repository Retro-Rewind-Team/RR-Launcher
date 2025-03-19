/*
    main.c - entry point and key init routines

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

#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include <wiisocket.h>
#include <ogc/wiilaunch.h>
#include <string.h>

#include "util.h"
#include "di.h"
#include "time.h"
#include "loader.h"
#include "dol.h"
#include "console.h"
#include "settings.h"

/* 100ms */
#define DISKCHECK_DELAY 100000

void *wiisocket_init_thread_callback(void *res)
{
    // Note: the void* given to us is an int* that lives in the main function,
    // because we want to assert everything from the main thread rather than asserting in here
    // so that we don't potentially exit(1) from another thread while the main thread is doing some important reading/patching.
    *(int *)res = wiisocket_init();
    rrc_dbg_printf("network initialised with status %d\n", *(int *)res);
    return NULL;
}

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void video_init()
{
    // Initialise the video system
    VIDEO_Init();

    // Obtain the preferred video mode from the system
    // This will correspond to the settings in the Wii menu
    rmode = VIDEO_GetPreferredMode(NULL);
    // Allocate memory for the display in the uncached region
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    // Initialise the console, required for printf
    console_init(xfb, 0, 0, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    rrc_con_set_line_width_chars(rmode->fbWidth / (sizeof(char) * 8 /* bits */));
    //  SYS_STDIO_Report(true);
    //  Set up the video registers with the chosen mode
    VIDEO_Configure(rmode);
    // Tell the video hardware where our display memory is
    VIDEO_SetNextFramebuffer(xfb);
    // Make the display visible
    VIDEO_SetBlack(false);
    // Flush the video register changes to the hardware
    VIDEO_Flush();
    // Wait for Video setup to complete
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();
}

int main(int argc, char **argv)
{
    s64 systime_start = gettime();
    // response codes for various library functions
    int res;

    // init video, setup console framebuffer
    video_init();

    rrc_con_update("Initialise controllers", 0);

    rrc_dbg_printf("init controllers\n");
    res = WPAD_Init();
    RRC_ASSERTEQ(res, WPAD_ERR_NONE, "WPAD_Init");

#define INTERRUPT_TIME 3000000 /* 3 seconds */
    rrc_con_clear(true);
    rrc_con_print_text_centered(_RRC_PRINTF_ROW, "Press A to launch, or press + to load settings.");
    rrc_con_print_text_centered(_RRC_PRINTF_ROW + 1, "Auto-launching in 3 seconds...");

    for (int i = 0; i < INTERRUPT_TIME / RRC_WPAD_LOOP_TIMEOUT; i++)
    {
        WPAD_ScanPads();

        int pressed = WPAD_ButtonsDown(0);
        if (pressed & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME))
        {
            return 0;
        }

        if (pressed & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A))
        {
            break;
        }

        if (pressed & (WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS))
        {
            switch (rrc_settings_display())
            {
            case RRC_SETTINGS_LAUNCH:
                goto interrupt_loop_end;
            case RRC_SETTINGS_EXIT:
                return 0;
            }
        }

        usleep(RRC_WPAD_LOOP_TIMEOUT);
    }
interrupt_loop_end:

    rrc_con_clear(true);

    rrc_con_update("Spawn background threads", 5);

    // Initializing the network can take fairly long (seconds).
    // It's not really needed right away anyway so we can do it on another thread in parallel to some of the disk reading
    // and join on it later when we actually need it.
    rrc_dbg_printf("spawn network init thread\n");
    int wiisocket_res;
    lwp_t wiisocket_thread;
    res = LWP_CreateThread(&wiisocket_thread, wiisocket_init_thread_callback, &wiisocket_res, NULL, 0, RRC_LWP_PRIO_IDLE);
    RRC_ASSERTEQ(res, RRC_LWP_OK, "LWP_CreateThread for wiisocket init");

    rrc_con_update("Initialise DVD", 10);

    rrc_dbg_printf("init disk drive\n");
    int fd = rrc_di_init();
    RRC_ASSERT(fd != 0, "rrc_di_init");

    rrc_con_update("Initialise DVD: Check for Mario Kart Wii", 12);
    /*  We should load Mario Kart Wii before doing anything else */
    res = rrc_loader_await_mkw();

    /*  TODO: From this point in the full launcher we will set a timeout of, say, 2 seconds.
        If some button such as A is pressed in that window, initialise the full channel.
        Otherwise, just go ahead and load the game. This saves the user time because on
        most occasions all you want to do is play and not do anything in the settings.

        For now, we're just loading the game. However, we need to do this in stages instead
        of in one big routine (like the WFC launcher). This is because while we're reading
        all of the necessary sections from disc, we're still initalising the network and
        fetching version information in the background thread. This thread will return
        version information as read from the API's text file, so when we join that thread,
        we compare those versions against our local version.txt and then ask the user if they
        want to update (if necessary). This replaces files on the SD card, so once all that
        is done, we can finally read patches from the SD, apply them, and load the game.
        So, all disc reading can be done in advance up to the point we read patch information
        from the SD.
    */

    // We've identified the game. Now find the data partition, which will tell us where the DOL and FST is.
    // This first requires parsing the partition *groups*. Each partition group contains multiple partitions.
    // Data partitions have the id 0.

    rrc_con_update("Initialise DVD: Load Data Partition", 15);
    struct rrc_di_part_group part_groups[4] __attribute__((aligned(32)));
    res = rrc_di_unencrypted_read(&part_groups, sizeof(part_groups), RRC_DI_PART_GROUPS_OFFSET >> 2);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_unencrypted_read for partition group");

    u32 data_part_offset = UINT32_MAX;
    res = rrc_loader_locate_data_part(&data_part_offset);

    if (data_part_offset == UINT32_MAX)
    {
        RRC_FATAL("no data partition found on disk");
    }
    rrc_dbg_printf("data partition found at offset %x\n", data_part_offset << 2);

    rrc_con_update("Initialise DVD: Read Data Partition", 17);

    res = rrc_di_open_partition(data_part_offset);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_open_partition");

    struct rrc_di_data_part_header data_header[3] __attribute__((aligned(32)));
    res = rrc_di_read(&data_header, sizeof(data_header), 0x420 >> 2);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_read data partition header");

    rrc_dbg_printf("DOL offset: %d\n", data_header->dol_offset << 2);
    rrc_dbg_printf("FST offset: %d\n", data_header->fst_offset << 2);
    rrc_dbg_printf("FST size: %d\n", data_header->fst_size << 2);

    rrc_con_update("Await Network", 20);

    res = LWP_JoinThread(wiisocket_thread, NULL);
    RRC_ASSERTEQ(res, RRC_LWP_OK, "LWP_JoinThread wiisocket init");
    RRC_ASSERTEQ(wiisocket_res, 0, "wiisocket_init");

    rrc_con_update("Initialise DVD: Read Game DOL", 25);

    // read dol
    struct rrc_dol *dol = (struct rrc_dol *)0x80901000;
    res = rrc_di_read(dol, sizeof(struct rrc_dol), data_header->dol_offset);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_read for dol");

    rrc_dbg_printf("Entrypoint at %x\n", dol->entry_point);
    rrc_dbg_printf("BSS Addr: %x\n", dol->bss_addr);
    rrc_dbg_printf("BSS size: %d\n", dol->bss_size);
    for (u32 i = 0; i < RRC_DOL_SECTION_COUNT; i++)
    {
        if (dol->section_size[i] == 0)
        {
            continue;
        }
        rrc_dbg_printf("%x at %x-%x (%d b)\n", dol->section[i], dol->section_addr[i], dol->section_addr[i] + dol->section_size[i], dol->section_size[i]);
        if ((dol->section_addr[i] < 0x80000000) || (dol->section_addr[i] + dol->section_size[i] > 0x90000000))
        {
            RRC_FATAL("Invalid section address: %x", dol->section_addr[i]);
        }

        // See patch.c comment for why we first copy them to `dol + dol->section[i]` rather than to `section_addr[i]` directly.
        res = rrc_di_read(
            (void *)((u32)dol + dol->section[i]),
            dol->section_size[i],
            data_header->dol_offset + (dol->section[i] >> 2));
        RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_read section");
    }

    rrc_con_update("Initialise DVD: Read Filesystem Table", 50);

    u32 mem1_hi = 0x81800000;
    u32 mem2_hi = *(u32 *)0x80003128;
    rrc_dbg_printf("mem1 hi: %x, mem2 hi %x\n", mem1_hi, mem2_hi);

    u32 fst_size = data_header->fst_size << 2;
    u32 fst_dest = align_down(mem1_hi - fst_size, 32);
    if (fst_dest < 0x81700000)
    {
        RRC_FATAL("fst size too large");
    }
    mem1_hi = fst_dest;
    rrc_dbg_printf("FST at %x, size: %d, aligned: %d\n", fst_dest, fst_size, align_up(fst_size, 32));
    res = rrc_di_read((void *)fst_dest, align_up(fst_size, 32), data_header->fst_offset);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_read fst");

    *((u32 *)0x80000038) = fst_dest; // start of FST

    // read BI2
    mem1_hi = align_down(mem1_hi - RRC_BI2_SIZE, 32);
    void *bi2 = (void *)(mem1_hi);
    res = rrc_di_read(bi2, RRC_BI2_SIZE, 0x440 >> 2);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_read for bi2");
    DCStoreRange(bi2, RRC_BI2_SIZE);

    rrc_con_update("Prepare For Patching", 60);

    // start shutting down background threads to boot the game

    WPAD_Shutdown();

    s64 systime_end = gettime();
    rrc_dbg_printf("time taken: %.3f seconds\n", ((f64)diff_msec(systime_start, systime_end)) / 1000.0);

    rrc_loader_load(dol, bi2, mem1_hi, mem2_hi);

    return 0;
}

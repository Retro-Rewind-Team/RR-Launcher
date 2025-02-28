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
#include "shutdown.h"
#include "di.h"
#include "time.h"

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

int main(int argc, char **argv)
{
    s64 systime_start = gettime();

    // response codes for various library functions
    int res;

    // Initialise the video system
    VIDEO_Init();

    // Obtain the preferred video mode from the system
    // This will correspond to the settings in the Wii menu
    rmode = VIDEO_GetPreferredMode(NULL);
    // Allocate memory for the display in the uncached region
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    // Initialise the console, required for printf
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    // SYS_STDIO_Report(true);
    // Set up the video registers with the chosen mode
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

    // seek to start
    printf("\x1b[0;0H");

    rrc_dbg_printf("spawn shutdown background thread\n");
    lwp_t shutdown_thread = rrc_shutdown_spawn();

    // Initializing the network can take fairly long (seconds).
    // It's not really needed right away anyway so we can do it on another thread in parallel to some of the disk reading
    // and join on it later when we actually need it.
    rrc_dbg_printf("spawn network init thread\n");
    int wiisocket_res;
    lwp_t wiisocket_thread;
    res = LWP_CreateThread(&wiisocket_thread, wiisocket_init_thread_callback, &wiisocket_res, NULL, 0, RRC_LWP_PRIO_IDLE);
    RRC_ASSERTEQ(res, RRC_LWP_OK, "LWP_CreateThread for wiisocket init");

    rrc_dbg_printf("init controllers\n");
    res = WPAD_Init();
    RRC_ASSERTEQ(res, WPAD_ERR_NONE, "WPAD_Init");

    rrc_dbg_printf("init disk drive\n");
    int fd = rrc_di_init();
    RRC_ASSERT(fd != 0, "rrc_di_init");

    unsigned int status;
    bool disc_printed = false;

check_cover_register:
    CHECK_EXIT();

    res = rrc_di_get_low_cover_register(&status);
    RRC_ASSERTEQ(res, RRC_DI_RET_OK, "rrc_di_getlowcoverregister");

    // if status = 0 that means that a disk is inserted
    if ((status & RRC_DI_DICVR_CVR) != 0)
    {
    missing_mkwii_alert:
        if (!disc_printed)
        {
            printf("Please insert Mario Kart Wii into the console.\n");
            disc_printed = true;
        }
        usleep(DISKCHECK_DELAY);
        goto check_cover_register;
    }

    /* we need to check we actually inserted mario kart wii */
    struct rrc_di_disk_id did;
    res = rrc_di_get_disk_id(&did);
    /* likely drive wasnt spun up */
    if (res != RRC_DI_LIBDI_EIO)
    {
        /* spin up the drive */
        rrc_dbg_printf("failed to read disk_id: attempting drive reset\n");
        RRC_ASSERTEQ(rrc_di_reset(), RRC_DI_LIBDI_OK, "rrc_di_reset");
        res = rrc_di_get_disk_id(&did);
        RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_get_disk_id (could not initialise drive)");
    }

    /* this excludes region identifier */
#define DISKID_MKW_ID "RMC"
    if (memcmp(did.game_id, DISKID_MKW_ID, strlen(DISKID_MKW_ID)))
        goto missing_mkwii_alert;

    char gameId[16];
    snprintf(
        gameId, sizeof(gameId), "%c%c%c%cD%02x", did.game_id[0],
        did.game_id[1], did.game_id[2], did.game_id[3], did.disc_ver);

    printf("Game ID/Rev: %s\n", gameId);
    CHECK_EXIT();

    // We've identified the game. Now find the data partition, which will tell us where the DOL and FST is.
    // This first requires parsing the partition *groups*. Each partition group contains multiple partitions.
    // Data partitions have the id 0.

    struct rrc_di_part_group part_groups[4] __attribute__((aligned(32)));
    res = rrc_di_unencrypted_read(&part_groups, sizeof(part_groups), RRC_DI_PART_GROUPS_OFFSET >> 2);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_unencrypted_read for partition group");

    struct rrc_di_part_info partitions[4] __attribute__((aligned(32)));
    struct rrc_di_part_info *data_part = NULL;

    for (u32 i = 0; i < 4 && data_part == NULL; i++)
    {
        if (part_groups[i].count == 0 && part_groups[i].offset == 0)
        {
            // No partitions in this group.
            continue;
        }

        if (part_groups[i].count > 4)
        {
            RRC_FATAL("too many partitions in group %d (max: 4, got: %d)", i, part_groups[i].count);
        }

        res = rrc_di_unencrypted_read(&partitions, sizeof(partitions), part_groups[i].offset);
        RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_unencrypted_read for partition");
        for (u32 j = 0; j < part_groups[i].count; j++)
        {
            if (partitions[j].type == RRC_DI_PART_TYPE_DATA)
            {
                data_part = &partitions[j];
                break;
            }
        }
    }

    if (data_part == NULL)
    {
        RRC_FATAL("no data partition found on disk");
    }
    printf("data partition found at offset %x\n", data_part->offset << 2);
    CHECK_EXIT();

    res = rrc_di_open_partition(data_part->offset);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_open_partition");

    struct rrc_di_data_part_header data_header[3] __attribute__((aligned(32)));
    res = rrc_di_read(&data_header, sizeof(data_header), 0x420 >> 2);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_read data partition header");

    printf("DOL offset: %d\n", data_header->dol_offset << 2);
    printf("FST offset: %d\n", data_header->fst_offset << 2);
    printf("FST size: %d\n", data_header->fst_size << 2);

    res = LWP_JoinThread(wiisocket_thread, NULL);
    RRC_ASSERTEQ(res, RRC_LWP_OK, "LWP_JoinThread wiisocket init");
    RRC_ASSERTEQ(wiisocket_res, 0, "wiisocket_init");

    s64 systime_end = gettime();
    rrc_dbg_printf("Time taken: %.3f seconds\n", ((f64)diff_msec(systime_start, systime_end)) / 1000.0);

    rrc_shutdown_join(shutdown_thread);

    return 0;
#undef CHECK_EXIT
}

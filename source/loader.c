/*
    loader.h - main app loader and patcher

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

#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <wiiuse/wpad.h>

#include "prompt.h"
#include "patch.h"
#include "util.h"
#include "di.h"
#include "loader.h"
#include "res.h"
#include "console.h"

int rrc_loader_locate_data_part(u32 *data_part_offset)
{
    int res;
    struct rrc_di_part_group part_groups[4] __attribute__((aligned(32)));
    res = rrc_di_unencrypted_read(&part_groups, sizeof(part_groups), RRC_DI_PART_GROUPS_OFFSET >> 2);
    RRC_ASSERTEQ(res, RRC_DI_LIBDI_OK, "rrc_di_unencrypted_read for partition group");

    struct rrc_di_part_info partitions[4] __attribute__((aligned(32)));

    for (u32 i = 0; i < 4 && *data_part_offset == UINT32_MAX; i++)
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
                *data_part_offset = partitions[j].offset;
                break;
            }
        }
    }

    return 0;
}

int rrc_loader_await_mkw(void *xfb)
{
    int res;
    unsigned int status;
    bool disc_printed = false;

check_cover_register:
    res = rrc_di_get_low_cover_register(&status);
    RRC_ASSERTEQ(res, RRC_DI_RET_OK, "rrc_di_getlowcoverregister");

    // if status = 0 that means that a disk is inserted
    if ((status & RRC_DI_DICVR_CVR) != 0)
    {
    missing_mkwii_alert:
        char *lines[] = {
            "Please insert Mario Kart Wii into the console,",
            "and select OK when done."};

        enum rrc_prompt_result pres = rrc_prompt_ok_cancel(xfb, lines, 2);
        RRC_ASSERT(pres != RRC_PROMPT_RESULT_ERROR, "failed to generate prompt");

        if (pres == RRC_PROMPT_RESULT_OK)
        {
            goto check_cover_register;
        }
        else
        {
            return RRC_RES_SHUTDOWN_INTERRUPT;
        }
    }

    rrc_dbg_printf("check disc");

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

    rrc_dbg_printf("Game ID/Rev: %s\n", gameId);

    return RRC_RES_OK;
}

void rrc_loader_load(void *dol, void *bi2_dest, u32 mem1_hi, u32 mem2_hi)
{
    rrc_con_update("Prepare For Patching: Patch Memory Map", 65);

    // Addresses are taken from <https://wiibrew.org/wiki/Memory_map> for the most part.

    *(u32 *)0xCD006C00 = 0x00000000;           // Reset `AI_CONTROL` to fix audio
    *(u32 *)0x80000034 = 0;                    // Arena High
    *(u32 *)0x800000EC = 0x81800000;           // Dev Debugger Monitor Address
    *(u32 *)0x800000F0 = 0x01800000;           // Simulated Memory Size
    *(u32 *)0x800000F4 = (u32)bi2_dest;        // Pointer to bi2
    *(u32 *)0x800000F8 = 0x0E7BE2C0;           // Console Bus Speed
    *(u32 *)0x800000FC = 0x2B73A840;           // Console CPU Speed
    *(u32 *)0x80003110 = mem1_hi;              // MEM1 Arena End
    *(u32 *)0x80003124 = 0x90000800;           // Usable MEM2 Start
    *(u32 *)0x80003128 = mem2_hi;              // Usable MEM2 End
    *(u32 *)0x80003180 = *(u32 *)(0x80000000); // Game ID
    *(u32 *)0x80003188 = *(u32 *)(0x80003140); // Minimum IOS Version

    if (*(u32 *)((u32)bi2_dest + 0x30) == 0x7ED40000)
    {
        *(u8 *)0x8000319C = 0x81; // Disc is dual layer
    }
    else
    {
        *(u8 *)0x8000319C = 0x80; // Disc is single layer
    }
    DCStoreRange((void *)0x80000000, 0x3400);
    ICInvalidateRange((void *)0x80000000, 0x3400);

    rrc_con_update("Patch and Launch Game", 75);

    // The last step is to copy the sections from the safe space to where they actually need to be.
    // This requires copying the function itself to the safe address space so we don't overwrite ourselves.
    // It also needs to call `DCFlushRange` but cannot reference it in the function, so we copy it and pass it as a function pointer.
    // See patch.c comment for a more detailed explanation.

    void (*patch_copy)(struct rrc_dol *, void (*)(void *, u32)) = (void (*)(struct rrc_dol *, void (*)(void *, u32)))RRC_PATCH_COPY_ADDRESS;

    memcpy(patch_copy, patch_dol, PATCH_DOL_LEN);
    DCFlushRange(patch_copy, PATCH_DOL_LEN);
    ICInvalidateRange(patch_copy, PATCH_DOL_LEN);

    void (*flush_range_copy)() = (void *)align_up(RRC_PATCH_COPY_ADDRESS + PATCH_DOL_LEN, 32);
    memcpy(flush_range_copy, ICInvalidateRange, 64);
    DCFlushRange(flush_range_copy, 64);
    ICInvalidateRange(flush_range_copy, 64);

    __IOS_ShutdownSubsystems();
    for (u32 i = 0; i < 32; i++)
    {
        IOS_CloseAsync(i, 0, 0);
    }

    IRQ_Disable();

    // Set the stack pointer to the safe address space so we don't overwrite local variables when copying sections.
    u32 new_sp = 0x808ffa00;
    asm volatile("mr 1, %0" : : "r"(new_sp) : "memory");
    patch_copy(dol, flush_range_copy);
}

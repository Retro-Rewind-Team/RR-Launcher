/*
    game_dol_loader.c - final game launching logic

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

/**
 * IMPORTANT: This file is very special in what you can do in here and will do the very last steps
 * of copying sections and launching the game.
 *
 * In particular, the game expects its sections to be written to specific addresses that overlap with those of the launcher code,
 * so naively reading the sections from the disc to where they need to be will not work as we would overwrite/corrupt the launcher code
 * in the middle of the section copy loop.
 *
 * To get around this, we copy the function to a "safe address space" that we know is (a) not used and (b) does not overlap with any sections.
 *
 * Caveats to keep in mind when interacting with this file/function:
 * - You cannot reference any symbols (this includes calling functions, although function pointers are fine),
 *   as they are compiled to branching to relative offsets, which won't be correct anymore when the function machine code is copied elsewhere.
 * - This file needs to be compiled with -f-nobuiltin so that memset/memcpy loops aren't compiled to memset calls and run into point 1.
 * - When calling patch_dol, you need to also set the stack pointer to the safe space so that local variables are not overwritten.
 *   (Ideally we would use __attribute__((naked)) so that the function can manage its own stack space but that's not available for PPC...)
 * - You must make sure that no other threads are running that share the address space.
 *
 * We also need to call `DCFlushRange()` to invalidate the data cache after copying sections, however since we cannot reference symbols,
 * we require the caller to pass it as a function pointer.
 *
 * This file is compiled separately to a .o file and linked together with the main code again so that we can know the function length ahead of time.
 */

#include <stdint.h>
#include "game_dol_loader.h"

void patch_dol(struct rrc_dol *dol, struct rrc_riivo_memory_patch *mem_patches, int mem_patch_count, void (*ic_invalidate_range)(void *, u32), void (*dc_flush_range)(void *, u32))
{
    // First, zero BSS.
    u64 *bss8 = (u64 *)dol->bss_addr;
    u8 *start = (u8 *)bss8;
    u8 *end = start + dol->bss_size;

    for (u8 *bss = (u8 *)bss8; bss < end; bss++)
    {
        *bss = 0;
    }

    dc_flush_range((void *)dol->bss_addr, dol->bss_size);
    ic_invalidate_range((void *)dol->bss_addr, dol->bss_size);

    // Next, copy all sections to where they need to be.
    for (u8 section_index = 0; section_index < RRC_DOL_SECTION_COUNT; section_index++)
    {
        u8 *from = (u8 *)dol + dol->section[section_index];
        u8 *to = (u8 *)dol->section_addr[section_index];
        u32 size = dol->section_size[section_index];

        for (u32 i = 0; i < size; i++)
        {
            to[i] = from[i];
        }

        dc_flush_range((void *)to, size);
        ic_invalidate_range((void *)to, size);
    }

    for (int i = 0; i < mem_patch_count; i++)
    {
        struct rrc_riivo_memory_patch *patch = &mem_patches[i];
        u32 *dest = (u32 *)patch->addr;
        u32 value = *dest;
        if (patch->original_init && patch->original != value)
        {
            // Original doesn't match, skip the patch.
            continue;
        }
        *dest = patch->value;
        dc_flush_range(dest, 32);
        ic_invalidate_range(dest, 32);
    }

    ((void (*)())dol->entry_point)();

    // We shouldn't really return from the entry_point call, but if for some reason it happens,
    // just loop because we really can't return from this function as we overwrote everything.
    while (1)
    {
    }
}

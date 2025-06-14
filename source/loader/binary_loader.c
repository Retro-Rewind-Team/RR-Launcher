/*
    binary_loader.c - loading of misc binaries

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

#include <dol.h>

#include "../util.h"
#include "binary_loader.h"

bool rrc_binary_find_section_by_addr(struct rrc_dol *dol, u32 addr, void **virt_addr, u32 *section_index)
{
    for (int i = 0; i < RRC_DOL_SECTION_COUNT; i++)
    {
        if (addr >= dol->section_addr[i] && addr < dol->section_addr[i] + dol->section_size[i])
        {
            u32 section_addr_offset = addr - dol->section_addr[i];
            *virt_addr = (void *)((u32)dol + dol->section[i] + section_addr_offset);
            *section_index = i;
            return true;
        }
    }
    return false;
}

struct rrc_result rrc_binary_load_pulsar_loader(struct rrc_dol *dol, void *real_loader_addr)
{
    void *virt_loader_addr;
    u32 _section_index;
    if (!rrc_binary_find_section_by_addr(dol, (u32)real_loader_addr, &virt_loader_addr, &_section_index))
    {
        RRC_FATAL("Pulsar loader address %x is not part of any game section", (u32)real_loader_addr);
    }

    void *original_addr = virt_loader_addr;

    FILE *loader_pul_file = fopen(RRC_LOADER_PUL_PATH, "r");
    if (!loader_pul_file)
    {
        return rrc_result_create_error_errno(errno, "Failed to open " RRC_LOADER_PUL_PATH);
    }

    int read = 0;
    while ((read = fread(virt_loader_addr, 1, 4096, loader_pul_file)))
    {
        virt_loader_addr += read;
    }
    rrc_invalidate_cache(original_addr, (u32)virt_loader_addr - (u32)original_addr);

    fclose(loader_pul_file);
    return rrc_result_success;
}

static void get_runtime_ext_path(char region, char *out)
{
    snprintf(out, 64, RRC_RUNTIME_EXT_BASE_PATH "-%c.dol", region);
}

void rrc_binary_load_runtime_ext(char region)
{
    char runtime_ext_path[64];
    get_runtime_ext_path(region, runtime_ext_path);

    FILE *patch_file = fopen(runtime_ext_path, "r");
    if (!patch_file)
    {
        char err[64];
        snprintf(err, sizeof(err), "Failed to open %s", runtime_ext_path);
        struct rrc_result res = rrc_result_create_error_errno(errno, err);
        rrc_result_error_check_error_fatal(res);
    }
    struct rrc_dol patch_dol;

    int read = fread((void *)&patch_dol, sizeof(patch_dol), 1, patch_file);
    if (read != 1)
    {
        fclose(patch_file);
        char err[64];
        snprintf(err, sizeof(err), "Failed to read %s", runtime_ext_path);
        struct rrc_result res = rrc_result_create_error_errno(errno, err);
        rrc_result_error_check_error_fatal(res);
    }

    memset((void *)patch_dol.bss_addr, 0, patch_dol.bss_size);
    rrc_invalidate_cache((void *)patch_dol.bss_addr, patch_dol.bss_size);

    for (int i = 0; i < RRC_DOL_SECTION_COUNT; i++)
    {
        u32 sec = patch_dol.section[i];
        u32 sec_addr = patch_dol.section_addr[i];
        u32 sec_size = patch_dol.section_size[i];
        if (sec_addr == 0)
            continue;

        if (sec_addr + sec_size > 0x817fffff)
        {
            RRC_FATAL("section %d overflows MEM1: %x + %x > 0x817fffff", i, sec_addr, sec_size);
        }

        if (fseek(patch_file, sec, SEEK_SET) != 0)
        {
            fclose(patch_file);
            char err[64];
            snprintf(err, sizeof(err), "Failed to seek to section %i in %s", sec, runtime_ext_path);
            struct rrc_result res = rrc_result_create_error_errno(errno, err);
            rrc_result_error_check_error_fatal(res);
        }

        if (fread((void *)sec_addr, sec_size, 1, patch_file) != 1)
        {
            fclose(patch_file);
            char err[64];
            snprintf(err, sizeof(err), "Failed to read section %i in %s", sec, runtime_ext_path);
            struct rrc_result res = rrc_result_create_error_errno(errno, err);
            rrc_result_error_check_error_fatal(res);
        }

        rrc_invalidate_cache((void *)sec_addr, sec_size);
    }

    fclose(patch_file);
}

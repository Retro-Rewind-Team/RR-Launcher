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
#include "exception.h"
#include "../shared/riivo.h"
#include "util.h"
#include <mxml.h>
#include <wiisocket.h>

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

check_cover_register:
    res = rrc_di_get_low_cover_register(&status);
    RRC_ASSERTEQ(res, RRC_DI_RET_OK, "rrc_di_getlowcoverregister");

    // if status = 0 that means that a disk is inserted
    if ((status & RRC_DI_DICVR_CVR) != 0)
    {
    missing_mkwii_alert:
        char *lines[] = {
            "Mario Kart Wii is not inserted!",
            "",
            "Please insert Mario Kart Wii into the console,",
            "and select OK when done."};

        enum rrc_prompt_result pres = rrc_prompt_ok_cancel(xfb, lines, 4);
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

static char *bump_alloc_string(u32 *arena, const char *src)
{
    int src_len = strlen(src);
    *arena -= src_len + 1;
    char *dest = (char *)*arena;
    memcpy(dest, src, src_len);
    dest[src_len] = '\0';
    return dest;
}

#define MAX_FILE_PATCHES 1000
#define MAX_MEMORY_PATCHES 128
#define MAX_ENABLED_SETTINGS (64)
static struct rrc_result append_patches_for_option(
    mxml_node_t *top,
    mxml_index_t *index,
    const char *name,
    int value,
    const char **patch_list,
    int *patch_count)
{
    if (value == 0)
    {
        // 0 = disabled, no patches to append
        return rrc_result_success;
    }

    mxmlIndexReset(index);
    for (mxml_node_t *option = mxmlIndexEnum(index); option != NULL; option = mxmlIndexEnum(index))
    {
        const char *option_name = mxmlElementGetAttr(option, "name");
        if (strcmp(option_name, name) == 0)
        {
            // Get the nth-1 (0 is the implicit disabled, handled at the top, does not exist in the XML) child (excluding whitespace nodes),
            // which is the selected option.
            mxml_node_t *selected_choice = mxmlFindElement(option, top, "choice", NULL, NULL, MXML_DESCEND_FIRST);
            for (int i = 0; selected_choice != NULL; selected_choice = mxmlGetNextSibling(selected_choice))
            {
                if (mxmlGetType(selected_choice) != MXML_ELEMENT)
                    continue;

                if (i == value - 1)
                {
                    break;
                }
                i++;
            }

            if (!selected_choice)
            {
                return rrc_result_create_error_corrupted_rr_xml("choice option has no children");
            }

            // The children of `selected_choice` are the patches. Append them.
            for (mxml_node_t *patch = mxmlFindElement(selected_choice, top, "patch", NULL, NULL, MXML_DESCEND_FIRST); patch != NULL; patch = mxmlGetNextSibling(patch))
            {
                if (mxmlGetType(patch) != MXML_ELEMENT)
                    continue;

                if (strcmp(mxmlGetElement(patch), "patch") != 0)
                    continue;

                const char *patch_name = mxmlElementGetAttr(patch, "id");
                if (!patch_name)
                {
                    return rrc_result_create_error_corrupted_rr_xml("<patch> without an id encountered");
                }

                // Append the patch name to the list.
                if (*patch_count >= MAX_ENABLED_SETTINGS)
                {
                    return rrc_result_create_error_corrupted_rr_xml("Attempted to enable more than " RRC_STRINGIFY(MAX_ENABLED_SETTINGS) " settings!");
                }
                patch_list[*patch_count] = patch_name;
                (*patch_count)++;
            }

            return rrc_result_success;
        }
    }

    return rrc_result_create_error_corrupted_rr_xml("option not found in xml");
}

struct parse_riivo_output
{
    struct rrc_riivo_memory_patch *mem_patches;
    int mem_patches_count;
    void *loader_pul_dest;
};

/**
 * Parses <file> and <folder> patches in the XML file and gives runtime-ext a pointer to it.
 * <memory> patches are also parsed
 */
static struct rrc_result parse_riivo_patches(struct rrc_settingsfile *settings, u32 *mem1, u32 *mem2, struct parse_riivo_output *out)
{
#define PARSE_REQUIRED_ATTR(node, var, attr)                                                                    \
    const char *var = mxmlElementGetAttr(node, attr);                                                           \
    if (!var)                                                                                                   \
    {                                                                                                           \
        return rrc_result_create_error_corrupted_rr_xml("missing " attr " attribute on " #node " replacement"); \
    }

    out->loader_pul_dest = NULL;

    u32 mem1_orig = *mem1;
    // Reserve space for file/folder replacements.
    *mem1 -= sizeof(struct rrc_riivo_disc_replacement) * MAX_FILE_PATCHES;
    *mem1 -= sizeof(struct rrc_riivo_disc);
    struct rrc_riivo_disc *riivo_disc = (void *)*mem1;
    // Reserve space for memory patches. Note: they don't actually need to be reserved in MEM1,
    // because it's only shortly needed in patch.c and never again at runtime.
    *mem1 -= sizeof(struct rrc_riivo_memory_patch) * MAX_MEMORY_PATCHES;
    out->mem_patches = (void *)*mem1;
    out->mem_patches_count = 0;

    // Read the XML to extract all possible options for the entries.
    FILE *
        xml_file = fopen(RRC_RIIVO_XML_PATH, "r");
    if (!xml_file)
    {
        return rrc_result_create_error_errno(errno, "Failed to open " RRC_RIIVO_XML_PATH);
    }

    mxml_node_t *xml_top = mxmlLoadFile(NULL, xml_file, NULL);

    const char *active_patches[MAX_ENABLED_SETTINGS];
    int active_patches_count = 0;

    mxml_index_t *options_index = mxmlIndexNew(xml_top, "option", NULL);

    append_patches_for_option(xml_top, options_index, "My Stuff", settings->my_stuff, active_patches, &active_patches_count);
    append_patches_for_option(xml_top, options_index, "Language", settings->language, active_patches, &active_patches_count);
    // Just always enable the pack, there is no setting for this.
    append_patches_for_option(xml_top, options_index, "Pack", RRC_SETTINGSFILE_PACK_ENABLED_VALUE, active_patches, &active_patches_count);

    // FIXME: Handle savegame options.

    // Iterate through <patch> elements.
    for (mxml_node_t *cur = mxmlFindElement(xml_top, xml_top, "patch", NULL, NULL, MXML_DESCEND_FIRST); cur != NULL; cur = mxmlGetNextSibling(cur))
    {
        if (riivo_disc->count >= MAX_FILE_PATCHES)
        {
            return rrc_result_create_error_corrupted_rr_xml("Attempted to enable more than " RRC_STRINGIFY(MAX_FILE_PATCHES) " file/folder replacements!");
        }

        if (mxmlGetType(cur) != MXML_ELEMENT)
            continue;

        if (strcmp(mxmlGetElement(cur), "patch") != 0)
            continue;

        // We have a <patch> element. Check if the id is an enabled setting, then process any of its contained <file> and <folder> elements.
        const char *elem_id = mxmlElementGetAttr(cur, "id");
        bool enabled = false;
        for (int i = 0; i < active_patches_count; i++)
        {
            if (strcmp(active_patches[i], elem_id) == 0)
            {
                enabled = true;
                break;
            }
        }
        if (!enabled)
            continue;

        mxml_index_t *file_repl_index = mxmlIndexNew(cur, "file", NULL);
        for (mxml_node_t *file = mxmlIndexEnum(file_repl_index); file != NULL; file = mxmlIndexEnum(file_repl_index))
        {
            PARSE_REQUIRED_ATTR(file, disc_path_mxml, "disc");
            PARSE_REQUIRED_ATTR(file, external_path_mxml, "external");

            char *disc_path_m1 = bump_alloc_string(mem1, disc_path_mxml);
            char *external_path_m1 = bump_alloc_string(mem1, external_path_mxml);

            struct rrc_riivo_disc_replacement *patch_dist = &riivo_disc->replacements[riivo_disc->count];
            patch_dist->disc = disc_path_m1;
            patch_dist->external = external_path_m1;
            patch_dist->type = RRC_RIIVO_FILE_REPLACEMENT;
            riivo_disc->count++;
        }
        mxmlIndexDelete(file_repl_index);

        mxml_index_t *folder_repl_index = mxmlIndexNew(cur, "folder", NULL);
        for (mxml_node_t *folder = mxmlIndexEnum(folder_repl_index); folder != NULL; folder = mxmlIndexEnum(folder_repl_index))
        {
            PARSE_REQUIRED_ATTR(folder, disc_path_mxml, "disc");
            // FIXME: this can/is actually sometimes be omitted and doesn't need to be required,
            // but this requires some special handling in the runtime-ext code to deal with
            PARSE_REQUIRED_ATTR(folder, external_path_mxml, "external");

            char *disc_path_m1 = bump_alloc_string(mem1, disc_path_mxml);
            char *external_path_m1 = external_path_mxml ? bump_alloc_string(mem1, external_path_mxml) : NULL;

            struct rrc_riivo_disc_replacement *patch_dist = &riivo_disc->replacements[riivo_disc->count];
            patch_dist->disc = disc_path_m1;
            patch_dist->external = external_path_m1;
            patch_dist->type = RRC_RIIVO_FOLDER_REPLACEMENT;
            riivo_disc->count++;
        }
        mxmlIndexDelete(folder_repl_index);

        mxml_index_t *memory_index = mxmlIndexNew(cur, "memory", NULL);
        for (mxml_node_t *memory = mxmlIndexEnum(memory_index); memory != NULL; memory = mxmlIndexEnum(memory_index))
        {
            PARSE_REQUIRED_ATTR(memory, addr_mxml, "offset");

            const char *valuefile_mxml = mxmlElementGetAttr(memory, "valuefile");
            // Bit of a hack, but in general we can't really handle valuefiles easily.
            // It would require loading an SD card file inside of the patch function
            // where we barely only have access to a single function.
            if (valuefile_mxml != NULL)
            {
                if (strcmp(valuefile_mxml, "/" RRC_LOADER_PUL_PATH) == 0)
                {
                    // Loader.pul specifically is handled manually elsewhere, so make an exception for this.
                    u32 loader_addr = strtoul(addr_mxml, NULL, 16);
                    out->loader_pul_dest = (void *)loader_addr;
                    continue;
                }

                return rrc_result_create_error_corrupted_rr_xml("Unhandled valuefile memory patch encountered");
            }

            PARSE_REQUIRED_ATTR(memory, value_mxml, "value");
            const char *original_mxml = mxmlElementGetAttr(memory, "original");

            struct rrc_riivo_memory_patch *patch_dist = &out->mem_patches[out->mem_patches_count];
            out->mem_patches_count++;
            patch_dist->addr = strtoul(addr_mxml, NULL, 16);
            patch_dist->value = strtoul(value_mxml, NULL, 16);
            patch_dist->original_init = false;
            if (original_mxml)
            {
                patch_dist->original = strtoul(original_mxml, NULL, 16);
                patch_dist->original_init = true;
            }
        }
        mxmlIndexDelete(memory_index);
    }

    // This address is a `static` in the runtime-ext dol that holds a pointer to the replacements, defined in the linker script.
    *((struct rrc_riivo_disc **)(RRC_RIIVO_DISC_PTR)) = riivo_disc;
    rrc_invalidate_cache((void *)*mem1, mem1_orig - *mem1);

    mxmlDelete(xml_top);
    fclose(xml_file);

    return rrc_result_success;
#undef REQUIRE_ATTR
}

/**
 * Finds a game section that contains the given address.
 * Sets `virt_addr` to the address of `addr` within the section in the DOL in safe space,
 * and sets `section_index` to the index of the section.
 */
static bool find_section_by_addr(struct rrc_dol *dol, u32 addr, void **virt_addr, u32 *section_index)
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

/**
 * Patches the DVD functions in the game DOL to immediately jump to custom DVD functions implemented in runtime-ext.
 * Also allocates trampolines containing the first 4 overwritten instructions + backjump to the original function,
 * which is called when the custom function wants to call the original DVD function.
 */
static void patch_dvd_functions(struct rrc_dol *dol)
{
    struct function_patch_entry
    {
        // Address of the function to patch.
        u32 addr;
        // Instructions to write at the end of the trampoline. This will jump back to the original DVD function + 16 (4 instructions).
        u32 backjmp_to_original[4];
        // Instructions to overwrite the start of the original DVD function with. This will jump to the custom function.
        u32 jmp_to_custom[4];
    };

    struct function_patch_entry entries[] = {
        {.addr = RRC_DVD_CONVERT_PATH_TO_ENTRYNUM, .backjmp_to_original = {0x3d208015, 0x6129df5c, 0x7d2903a6, 0x4e800420}, .jmp_to_custom = {0x3d208178, 0x61292e60, 0x7d2903a6, 0x4e800420}},
        {.addr = RRC_DVD_FAST_OPEN, .backjmp_to_original = {0x3d208015, 0x6129e264, 0x7d2903a6, 0x4e800420}, .jmp_to_custom = {0x3d208178, 0x61292ee0, 0x7d2903a6, 0x4e800420}},
        {.addr = RRC_DVD_OPEN, .backjmp_to_original = {0x3d208015, 0x6129e2cc, 0x7d2903a6, 0x4e800420}, .jmp_to_custom = {0x3d208178, 0x61292ea0, 0x7d2903a6, 0x4e800420}},
        {.addr = RRC_DVD_READ_PRIO, .backjmp_to_original = {0x3d208015, 0x6129e844, 0x7d2903a6, 0x4e800420}, .jmp_to_custom = {0x3d208178, 0x61292f20, 0x7d2903a6, 0x4e800420}},
        {.addr = RRC_DVD_CLOSE, .backjmp_to_original = {0x3d208015, 0x6129e578, 0x7d2903a6, 0x4e800420}, .jmp_to_custom = {0x3d208178, 0x61292f60, 0x7d2903a6, 0x4e800420}},
    };

    for (int i = 0; i < sizeof(entries) / sizeof(struct function_patch_entry); i++)
    {
        struct function_patch_entry entry = entries[i];
        if (entry.addr == 0)
            continue;

        u32 section_index;
        void *virt_addr;
        if (!find_section_by_addr(dol, entry.addr, &virt_addr, &section_index))
        {
            RRC_FATAL("Address to patch %x is not part of any game section", entry.addr);
        }

        // 32 bytes (4 instructions for the backjmp + 4 overwritten instructions restored) per patched function.
        // This is the start of the trampoline.
        u32 *hooked_addr = (u32 *)(0x93400000 + (i * 32));
        // Prepare the trampoline: copy the first 4 instructions of the original function that we're about to overwrite to the start,
        // and append the `backjmp_to_original` instructions.
        memcpy(hooked_addr, virt_addr, 16);
        memcpy(hooked_addr + 4, entry.backjmp_to_original, 16);
        rrc_invalidate_cache(hooked_addr, 32);

        // Overwrite the original function with a jump to the custom DVD function.
        memcpy(virt_addr, entry.jmp_to_custom, 16);
        rrc_invalidate_cache(virt_addr, 16);
    }
}

static struct rrc_result load_pulsar_loader(struct rrc_dol *dol, void *real_loader_addr)
{
    void *virt_loader_addr;
    u32 _section_index;
    if (!find_section_by_addr(dol, (u32)real_loader_addr, &virt_loader_addr, &_section_index))
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

static struct rrc_result load_runtime_ext(const char *apps_cwd)
{
    // Create the path relative to the cwd (sd:// on dolphin or apps folder through HBC).
    char dol_path[PATH_MAX];
    char *path_ptr = dol_path;
    int cwd_len = strlen(apps_cwd);
    RRC_ASSERT(cwd_len < PATH_MAX, "apps_cwd longer than PATH_MAX");

    strncpy(path_ptr, apps_cwd, sizeof(dol_path));
    path_ptr += cwd_len;

    if (dol_path[cwd_len - 1] != '/')
    {
        // Add a trailing slash if it doesn't exist already.
        dol_path[cwd_len] = '/';
        path_ptr++;
    }
    strncpy(path_ptr, "runtime-ext.dol", sizeof(dol_path) - (path_ptr - dol_path));

    FILE *patch_file = fopen(dol_path, "r");
    if (!patch_file)
    {
        return rrc_result_create_error_errno(errno, "Failed to open runtime-ext.dol");
    }
    struct rrc_dol patch_dol;

    int read = fread((void *)&patch_dol, sizeof(patch_dol), 1, patch_file);
    if (read != 1)
    {
        fclose(patch_file);
        return rrc_result_create_error_errno(errno, "Failed to read full runtime-ext.dol");
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
            return rrc_result_create_error_errno(errno, "Failed to seek to section in runtime-ext.dol");
        }

        if (fread((void *)sec_addr, sec_size, 1, patch_file) != 1)
        {
            fclose(patch_file);
            return rrc_result_create_error_errno(errno, "Failed to read section in runtime-ext.dol");
        }

        rrc_invalidate_cache((void *)sec_addr, sec_size);
    }

    fclose(patch_file);
    return rrc_result_success;
}

typedef void (*patch_dol_func_t)(struct rrc_dol *, struct rrc_riivo_memory_patch *, int, void (*)(void *, u32), void (*)(void *, u32));

/**
 * Wrapper function around `patch_dol` that sets up the stack pointer to a safe location (workaround for missing support for __attribute__((naked))).
 */
void patch_dol_helper(
    /* r3 */ struct rrc_dol *dol,
    /* r4 */ struct rrc_riivo_memory_patch *mem_patches,
    /* r5 */ int mem_patch_count,
    /* r6 */ void (*ic_invalidate_range)(void *, u32),
    /* r7 */ void (*dc_flush_range)(void *, u32),
    /* r8 */ patch_dol_func_t);

asm("patch_dol_helper:\n"
    // Adjust the stack pointer to 0x808ffa00 (arbitrary, temporary, random safe address not used by game sections)
    // so we don't overwrite local variables while copying sections.
    "lis 9, -32625\n"
    "ori 9, 9, 64000\n"
    "mr 1,9\n"
    // Jump to the function in r8 (patch_dol). All other arguments are already in the right registers (r3-r7).
    "mtctr 8\n"
    "bctrl\n");

void rrc_loader_load(struct rrc_dol *dol, struct rrc_settingsfile *settings, const char *apps_cwd, void *bi2_dest, u32 mem1_hi, u32 mem2_hi)
{
    struct rrc_result res;

    // runtime-ext needs to be loaded before parsing riivo patches, as it writes to a static.
    // All errors that happen here are fatal; we can't boot the game without knowing the patches or having the patched DVD functions.
    res = load_runtime_ext(apps_cwd);
    rrc_result_error_check_error_fatal(&res);

    struct parse_riivo_output riivo_out;
    res = parse_riivo_patches(settings, &mem1_hi, &mem2_hi, &riivo_out);
    rrc_result_error_check_error_fatal(&res);

    patch_dvd_functions(dol);
    res = load_pulsar_loader(dol, riivo_out.loader_pul_dest);
    rrc_result_error_check_error_fatal(&res);

    rrc_con_update("Prepare For Patching: Patch Memory Map", 65);

    // Addresses are taken from <https://wiibrew.org/wiki/Memory_map> for the most part.

    *(u32 *)0xCD006C00 = 0x00000000;              // Reset `AI_CONTROL` to fix audio
    *(u32 *)0x80000034 = 0;                       // Arena High
    *(u32 *)0x800000EC = 0x81800000;              // Dev Debugger Monitor Address
    *(u32 *)0x800000F0 = 0x01800000;              // Simulated Memory Size
    *(u32 *)0x800000F4 = (u32)bi2_dest;           // Pointer to bi2
    *(u32 *)0x800000F8 = 0x0E7BE2C0;              // Console Bus Speed
    *(u32 *)0x800000FC = 0x2B73A840;              // Console CPU Speed
    *(u32 *)0x80003110 = align_down(mem1_hi, 32); // MEM1 Arena End
    *(u32 *)0x80003124 = 0x90000800;              // Usable MEM2 Start
    *(u32 *)0x80003128 = align_down(mem2_hi, 32); // Usable MEM2 End
    *(u32 *)0x80003180 = *(u32 *)(0x80000000);    // Game ID
    *(u32 *)0x80003188 = *(u32 *)(0x80003140);    // Minimum IOS Version

    memcpy((u32 *)0x80000000, "RMCP01", 6);
    DCFlushRange((u32 *)0x80000000, 32);
    memcpy((u32 *)0x80003180, "RMCP", 4);
    DCFlushRange((u32 *)0x80003180, 32);

    if (*(u32 *)((u32)bi2_dest + 0x30) == 0x7ED40000)
    {
        *(u8 *)0x8000319C = 0x81; // Disc is dual layer
    }
    else
    {
        *(u8 *)0x8000319C = 0x80; // Disc is single layer
    }
    ICInvalidateRange((void *)0x80000000, 0x3400);
    DCFlushRange((void *)0x80000000, 0x01800000);

    rrc_con_update("Patch and Launch Game", 75);

    wiisocket_deinit();

    // The last step is to copy the sections from the safe space to where they actually need to be.
    // This requires copying the function itself to the safe address space so we don't overwrite ourselves.
    // It also needs to call `DCFlushRange` but cannot reference it in the function, so we copy it and pass it as a function pointer.
    // See patch.c comment for a more detailed explanation.

    patch_dol_func_t patch_copy = (void *)RRC_PATCH_COPY_ADDRESS;

    memcpy(patch_copy, patch_dol, PATCH_DOL_LEN);
    DCFlushRange(patch_copy, align_up(PATCH_DOL_LEN, 32));
    ICInvalidateRange(patch_copy, align_up(PATCH_DOL_LEN, 32));

    void (*ic_invalidate_range)() = (void *)align_up(RRC_PATCH_COPY_ADDRESS + PATCH_DOL_LEN, 32);
    memcpy(ic_invalidate_range, ICInvalidateRange, 64);
    DCFlushRange(ic_invalidate_range, 64);
    ICInvalidateRange(ic_invalidate_range, 64);

    void (*dc_flush_range)() = (void *)align_up(RRC_PATCH_COPY_ADDRESS + PATCH_DOL_LEN + 64, 32);
    memcpy(dc_flush_range, DCFlushRange, 64);
    DCFlushRange(dc_flush_range, 64);
    ICInvalidateRange(dc_flush_range, 64);

    __IOS_ShutdownSubsystems();
    for (u32 i = 0; i < 32; i++)
    {
        IOS_Close(i);
    }

    IRQ_Disable();

    patch_dol_helper(
        dol,
        riivo_out.mem_patches,
        riivo_out.mem_patches_count,
        ic_invalidate_range,
        dc_flush_range,
        patch_copy);
}

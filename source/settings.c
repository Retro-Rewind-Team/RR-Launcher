/*
    settings.h - the settings menu implementation when auto-launch is interrupted by pressing +

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

#include "settings.h"
#include "util.h"
#include "console.h"
#include "settingsfile.h"
#include "update/update.h"
#include "prompt.h"
#include <stdio.h>
#include <string.h>
#include <gctypes.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <errno.h>
#include <mxml.h>

enum settings_entry_type
{
    ENTRY_TYPE_SELECT,
    ENTRY_TYPE_BUTTON
};

struct settings_entry
{
    /** The type of entry. */
    enum settings_entry_type type;

    /** Any additional newlines to print above (used to divide sections, i.e. having "Launch" and "Exit" separated by two lines) */
    u8 margin_top;

    /**
     * The initial selected option after the saved settings were loaded or saved again. Used for checking whether we have unsaved changes at any point.
     */
    u32 initial_selected_option;

    /**
     * A pointer that stores the option that is currently selected (index into `options`), iff this is a select option (otherwise NULL).
     * This is typically a pointer into a settingsfile struct field so that it automatically gets synchronized.
     */
    u32 *selected_option;

    /** The label or "name" of this setting to be displayed. */
    char *label;

    /** An array of possible selectable option names, if this is a select option (otherwise NULL). */
    const char **options;

    /** Number of selectable options. Must not be 0 if this is a selectable. */
    int option_count;
};

static char *launch_label = "Launch";
static char *my_stuff_label = "My Stuff";
static char *save_label = "Save changes";
static char *language_label = "Language";
static char *savegame_label = "Separate savegame";
static char *autoupdate_label = "Automatic updates";
static char *perform_updates_label = "Perform updates";
static char *changes_saved_status = RRC_CON_ANSI_FG_GREEN "Changes saved." RRC_CON_ANSI_CLR;
static char *exit_label = "Exit";

static void xml_find_option_choices(mxml_node_t *node, mxml_node_t *top, const char *name, const char ***result_choice, int *result_choice_count, u32 *saved_value)
{
    mxml_node_t *option_node = mxmlFindElement(node, top, "option", "name", name, MXML_DESCEND);
    RRC_ASSERT(option_node != NULL, "malformed RetroRewind6.xml: missing option in xml");

    mxml_index_t *index = mxmlIndexNew(option_node, "choice", "name");
    RRC_ASSERT(index != NULL, "failed to create index");

    int count = mxmlIndexGetCount(index);
    const char **out = malloc(sizeof(char *) * (count + 1 /* + implicit 'disabled' */));
    out[0] = "Disabled";

    mxml_node_t *choice;
    int i = 1;
    while ((choice = mxmlIndexEnum(index)) != NULL)
    {
        const char *choice_s = mxmlElementGetAttr(choice, "name");
        RRC_ASSERT(choice_s != NULL, "malformed RetroRewind6.xml: choice has no name attribute");

        RRC_ASSERT(i < count + 1, "index has more elements than choices");
        out[i++] = choice_s;
    }

    mxmlIndexDelete(index);
    *result_choice = out;
    *result_choice_count = i;

    // Clamp it in case the saved version is out of bounds.
    if (*saved_value < 0 || *saved_value >= *result_choice_count)
    {
        *saved_value = RRC_SETTINGSFILE_DEFAULT;
    }
}

static bool prompt_save_unsaved_changes(void *xfb, const struct settings_entry *entries, int entry_count)
{
    char *lines[] = {
        "There are unsaved changes.\n",
        "Would you like to save before exiting settings?"};

    enum rrc_prompt_result result = rrc_prompt_yes_no(xfb, lines, sizeof(lines) / sizeof(char *));
    return result == RRC_PROMPT_RESULT_YES;
}

#define CLEANUP             \
    free(my_stuff_options); \
    free(language_options); \
    free(savegame_options); \
    mxmlDelete(xml_top);    \
    fclose(xml_file);

enum rrc_settings_result rrc_settings_display(void *xfb, struct rrc_settingsfile *stored_settings)
{
    // Read the XML to extract all possible options for the entries.
    FILE *xml_file = fopen("RetroRewind6/xml/RetroRewind6.xml", "r");
    if (!xml_file)
    {
        RRC_FATAL("failed to open RetroRewind6.xml file: %d", errno);
    }
    mxml_node_t *xml_top = mxmlLoadFile(NULL, xml_file, NULL);

    mxml_node_t *xml_options = mxmlFindElement(xml_top, xml_top, "options", NULL, NULL, MXML_DESCEND);
    RRC_ASSERT(xml_options != NULL, "no <options> tag in xml");

    const char **my_stuff_options, **language_options, **savegame_options;
    int my_stuff_options_count, language_options_count, savegame_options_count;
    const char *autoupdate_options[] = {"Disabled", "Enabled"};
    int autoupdate_option_count = sizeof(autoupdate_options) / sizeof(char *);

    xml_find_option_choices(xml_options, xml_top, "My Stuff", &my_stuff_options, &my_stuff_options_count, &stored_settings->my_stuff);
    xml_find_option_choices(xml_options, xml_top, "Language", &language_options, &language_options_count, &stored_settings->language);
    xml_find_option_choices(xml_options, xml_top, "Seperate Savegame", &savegame_options, &savegame_options_count, &stored_settings->savegame);

    // Begin initializing the settings UI.
    rrc_con_clear(true);

    struct settings_entry entries[] = {
        {.type = ENTRY_TYPE_BUTTON, .label = launch_label},

        {.type = ENTRY_TYPE_SELECT,
         .label = my_stuff_label,
         .options = my_stuff_options,
         .selected_option = &stored_settings->my_stuff,
         .initial_selected_option = stored_settings->my_stuff,
         .option_count = my_stuff_options_count,
         .margin_top = 1},
        {.type = ENTRY_TYPE_SELECT,
         .label = language_label,
         .options = language_options,
         .selected_option = &stored_settings->language,
         .initial_selected_option = stored_settings->language,
         .option_count = language_options_count},
        {.type = ENTRY_TYPE_SELECT,
         .label = savegame_label,
         .options = savegame_options,
         .selected_option = &stored_settings->savegame,
         .initial_selected_option = stored_settings->savegame,
         .option_count = savegame_options_count},
        {.type = ENTRY_TYPE_SELECT,
         .label = autoupdate_label,
         .options = autoupdate_options,
         .selected_option = &stored_settings->auto_update,
         .initial_selected_option = stored_settings->auto_update,
         .option_count = autoupdate_option_count},

        {.type = ENTRY_TYPE_BUTTON, .label = save_label, .margin_top = 1},
        {.type = ENTRY_TYPE_BUTTON, .label = perform_updates_label},

        {.type = ENTRY_TYPE_BUTTON, .label = exit_label, .margin_top = 1},
    };
    const int entry_count = sizeof(entries) / sizeof(struct settings_entry);
    int selected_idx = 0;

    char status_message[64] = "";

    // Used for padding the label string with spaces so that all options are aligned with each other.
    u32 max_label_len = 0;
    for (int i = 0; i < entry_count; i++)
    {
        // Do some minimal validation on select settings while we have to go through all entries to get the max label length anyway.
        if (entries[i].type == ENTRY_TYPE_SELECT && entries[i].options == NULL)
        {
            RRC_FATAL("'%s' is a select option but has a NULL pointer for its options array", entries[i].label)
        }

        if (entries[i].type == ENTRY_TYPE_SELECT && entries[i].option_count == 0)
        {
            RRC_FATAL("'%s' is a select option but has 0 options to select", entries[i].label);
        }

        if (entries[i].type == ENTRY_TYPE_SELECT && entries[i].selected_option == NULL)
        {
            RRC_FATAL("'%s' is a select option but has a NULL selected_option pointer (likely not initialized)", entries[i].label);
        }

        int len = strlen(entries[i].label);
        if (len > max_label_len)
        {
            max_label_len = len;
        }
    }

    while (1)
    {
        int row = _RRC_SPLASH_ROW + 2;
        bool has_unsaved_changes = false;

        for (int i = 0; i < entry_count; i++)
        {
            const struct settings_entry *entry = &entries[i];

            // add any extra "newlines" (which means just seek)
            row += entry->margin_top;
            rrc_con_clear_line(row);
            rrc_con_cursor_seek_to(row, 0);

            bool is_selected = selected_idx == i;

            if (is_selected)
            {
                printf(RRC_CON_ANSI_FG_BRIGHT_WHITE);
                printf(">> ");
            }
            else
            {
                printf("   ");
            }

            printf("%s  ", entry->label);

            if (entry->type == ENTRY_TYPE_SELECT)
            {
                int label_len = strlen(entry->label);
                if (label_len < max_label_len)
                {
                    for (int i = 0; i < max_label_len - label_len; i++)
                    {
                        putc(' ', stdout);
                    }
                }

                if (is_selected)
                {
                    printf("> ");
                }

                printf("%s", entry->options[*entry->selected_option]);

                if (is_selected)
                {
                    printf(" <");
                }

                if (*entry->selected_option != entry->initial_selected_option)
                {
                    has_unsaved_changes = true;
                    printf("%s *", RRC_CON_ANSI_FG_WHITE);
                }
            }

            // end
            printf(RRC_CON_ANSI_CLR);
            row++;
        }

        row += 2;
        rrc_con_cursor_seek_to(row++, strlen(">> "));
        printf("Use the D-Pad to navigate.");

        if (has_unsaved_changes && strcmp(status_message, changes_saved_status) == 0)
        {
            // Reset the "changes saved" status message if we have unsaved changes.
            status_message[0] = 0;
        }

        rrc_con_clear_line(row);
        rrc_con_cursor_seek_to(row++, strlen(">> "));
        printf("%s", status_message);

        // use an inner loop just for scanning for button presses, rather than re-printing everything all the time
        // because the current scene will remain "static" until a button is pressed
        while (1)
        {
            WPAD_ScanPads();
            int pressed = WPAD_ButtonsDown(0);
            if (pressed & RRC_WPAD_HOME_MASK)
            {
                goto exit;
            }

            if (pressed & RRC_WPAD_DOWN_MASK)
            {
                if (selected_idx < entry_count - 1)
                {
                    selected_idx++;
                }
                else
                {
                    selected_idx = 0;
                }
                break;
            }

            if (pressed & RRC_WPAD_UP_MASK)
            {
                if (selected_idx > 0)
                {
                    selected_idx--;
                }
                else
                {
                    selected_idx = entry_count - 1;
                }
                break;
            }

            struct settings_entry *entry = &entries[selected_idx];

            if ((pressed & RRC_WPAD_LEFT_MASK) && entry->type == ENTRY_TYPE_SELECT)
            {
                if (*entry->selected_option > 0)
                {
                    (*entry->selected_option)--;
                }
                else
                {
                    // user pressed left while already at the last option, wrap back to the end
                    *entry->selected_option = entry->option_count - 1;
                }
                break;
            }

            if ((pressed & RRC_WPAD_RIGHT_MASK) && entry->type == ENTRY_TYPE_SELECT)
            {
                (*entry->selected_option)++;

                if (*entry->selected_option >= entry->option_count)
                {
                    // user pressed right while already at the last option, wrap back to the first one
                    *entry->selected_option = 0;
                }
                break;
            }

            if (pressed & RRC_WPAD_A_MASK)
            {
                if (entry->label == launch_label)
                {
                    if (has_unsaved_changes && prompt_save_unsaved_changes(xfb, entries, entry_count))
                    {
                        struct rrc_result res = rrc_settingsfile_store(stored_settings);
                        rrc_result_error_check_error_normal(&res, xfb);
                    }

                    goto launch;
                }
                else if (entry->label == save_label)
                {
                    struct rrc_result res = rrc_settingsfile_store(stored_settings);
                    rrc_result_error_check_error_normal(&res, xfb);

                    for (int i = 0; i < entry_count; i++)
                    {
                        if (entries[i].type == ENTRY_TYPE_SELECT)
                        {
                            entries[i].initial_selected_option = *entries[i].selected_option;
                        }
                    }

                    strncpy(status_message, changes_saved_status, sizeof(status_message));
                    break;
                }
                else if (entry->label == perform_updates_label)
                {
                    int update_count;
                    bool updated = rrc_update_do_updates(xfb, &update_count);
                    if (update_count == 0)
                    {
                        strncpy(status_message, "No updates available.", sizeof(status_message));
                    }
                    else if (updated)
                    {
                        snprintf(status_message, sizeof(status_message), "%d updates installed.", update_count);
                    }

                    rrc_con_clear(true);
                    break;
                }
                else if (entry->label == exit_label)
                {
                    if (has_unsaved_changes && prompt_save_unsaved_changes(xfb, entries, entry_count))
                    {
                        struct rrc_result res = rrc_settingsfile_store(stored_settings);
                        rrc_result_error_check_error_normal(&res, xfb);
                    }

                    goto exit;
                }
            }

            usleep(RRC_WPAD_LOOP_TIMEOUT);
        }
    }

    goto launch;

launch:
    CLEANUP
    return RRC_SETTINGS_LAUNCH;

exit:
    CLEANUP
    return RRC_SETTINGS_EXIT;
}

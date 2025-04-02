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

enum settings_option_type
{
    OPTION_TYPE_SELECT,
    OPTION_TYPE_BUTTON
};

struct settings_option
{
    /** The type of option. */
    enum settings_option_type type;
    /** Any additional newlines to print above (used to divide sections, i.e. having "Launch" and "Exit" separated by two lines) */
    u8 margin_top;
    /** The option (index into `options`) that is currently selected, in case that this is a select option. */
    u8 selected_option;
    /** The label or "name" of this setting to be displayed. */
    char *label;
    /** Number of selectable options. Must not be 0 if this is a selectable. */
    int option_count;
    /** An array of possible selectable option names, if this is a select option. */
    const char **options;
};

static char *launch_label = "Launch";
static char *my_stuff_label = "My Stuff";
static char *save_label = "Save changes";
static char *language_label = "Language";
static char *savegame_label = "Separate savegame";
static char *perform_updates_label = "Perform updates";
static char *exit_label = "Exit";

static void xml_find_option_choices(mxml_node_t *node, mxml_node_t *top, const char *name, const char ***result_choice, int *result_choice_count, int *saved_value)
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

#define CLEANUP             \
    free(my_stuff_options); \
    free(language_options); \
    free(savegame_options); \
    mxmlDelete(xml_top);    \
    fclose(xml_file);

enum rrc_settings_result rrc_settings_display(void *xfb)
{
    FILE *xml_file = fopen("RetroRewind6/xml/RetroRewind6.xml", "r");
    if (!xml_file)
    {
        RRC_FATAL("failed to open RetroRewind6.xml file: %d", errno);
    }
    mxml_node_t *xml_top = mxmlLoadFile(NULL, xml_file, NULL);

    struct rrc_settingsfile stored_settings;
    RRC_ASSERTEQ(rrc_settingsfile_parse(&stored_settings), RRC_SETTINGSFILE_OK, "failed to parse settings file");

    rrc_con_clear(true);

    mxml_node_t *xml_options = mxmlFindElement(xml_top, xml_top, "options", NULL, NULL, MXML_DESCEND);
    RRC_ASSERT(xml_options != NULL, "no <options> tag in xml");

    const char **my_stuff_options, **language_options, **savegame_options;
    int my_stuff_options_count, language_options_count, savegame_options_count;

    xml_find_option_choices(xml_options, xml_top, "My Stuff", &my_stuff_options, &my_stuff_options_count, &stored_settings.my_stuff);
    xml_find_option_choices(xml_options, xml_top, "Language", &language_options, &language_options_count, &stored_settings.language);
    xml_find_option_choices(xml_options, xml_top, "Seperate Savegame", &savegame_options, &savegame_options_count, &stored_settings.savegame);

    struct settings_option options[] = {
        {.type = OPTION_TYPE_BUTTON, .label = launch_label},
        {
            .type = OPTION_TYPE_SELECT,
            .label = my_stuff_label,
            .options = my_stuff_options,
            .selected_option = stored_settings.my_stuff,
            .option_count = my_stuff_options_count,
            .margin_top = 1,
        },
        {.type = OPTION_TYPE_SELECT, .label = language_label, .options = language_options, .selected_option = stored_settings.language, .option_count = language_options_count},
        {.type = OPTION_TYPE_SELECT, .label = savegame_label, .options = savegame_options, .selected_option = stored_settings.savegame, .option_count = savegame_options_count},
        {.type = OPTION_TYPE_BUTTON, .label = save_label, .margin_top = 1},
        {.type = OPTION_TYPE_BUTTON, .label = perform_updates_label},
        {.type = OPTION_TYPE_BUTTON, .label = exit_label, .margin_top = 1},
    };
    const int option_count = sizeof(options) / sizeof(struct settings_option);
    int selected_idx = 0;

    // used for padding the label string with spaces so that all options are aligned with each other
    u32 max_label_len = 0;
    for (int i = 0; i < option_count; i++)
    {
        if (options[i].type == OPTION_TYPE_SELECT && options[i].options == NULL)
        {
            RRC_FATAL("'%s' is a select option but has a NULL pointer for its options array", options[i].label)
        }

        if (options[i].type == OPTION_TYPE_SELECT && options[i].option_count == 0)
        {
            RRC_FATAL("'%s' is a select option but has 0 options to select", options[i].label);
        }

        int len = strlen(options[i].label);
        if (len > max_label_len)
        {
            max_label_len = len;
        }
    }

    while (1)
    {
        int row = _RRC_SPLASH_ROW + 2;
        for (int i = 0; i < option_count; i++)
        {
            const struct settings_option *option = &options[i];

            // add any extra "newlines" (which means just seek)
            row += option->margin_top;
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

            printf("%s  ", option->label);

            if (option->type == OPTION_TYPE_SELECT)
            {
                int label_len = strlen(option->label);
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

                printf("%s", option->options[option->selected_option]);

                if (is_selected)
                {
                    printf(" <");
                }
            }

            // end
            printf(RRC_CON_ANSI_CLR);
            row++;
        }

        row += 2;
        rrc_con_cursor_seek_to(row, strlen(">> "));
        printf("Use the D-Pad to navigate.");

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
                if (selected_idx < option_count - 1)
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
                    selected_idx = option_count - 1;
                }
                break;
            }

            struct settings_option *option = &options[selected_idx];

            if ((pressed & RRC_WPAD_LEFT_MASK) && option->type == OPTION_TYPE_SELECT)
            {
                if (option->selected_option > 0)
                {
                    option->selected_option--;
                }
                else
                {
                    // user pressed left while already at the last option, wrap back to the end
                    option->selected_option = option->option_count - 1;
                }
                break;
            }

            if ((pressed & RRC_WPAD_RIGHT_MASK) && option->type == OPTION_TYPE_SELECT)
            {
                option->selected_option++;

                if (option->selected_option >= option->option_count)
                {
                    // user pressed right while already at the last option, wrap back to the first one
                    option->selected_option = 0;
                }
                break;
            }

            if (pressed & RRC_WPAD_A_MASK)
            {
                if (option->label == launch_label)
                {
                    goto launch;
                }
                else if (option->label == save_label)
                {
                    for (int i = 0; i < sizeof(options) / sizeof(struct settings_option); i++)
                    {
                        if (options[i].label == my_stuff_label)
                        {
                            stored_settings.my_stuff = options[i].selected_option;
                        }
                        else if (options[i].label == language_label)
                        {
                            stored_settings.language = options[i].selected_option;
                        }
                        else if (options[i].label == savegame_label)
                        {
                            stored_settings.savegame = options[i].selected_option;
                        }
                    }
                    rrc_settingsfile_store(&stored_settings);
                }
                else if (option->label == perform_updates_label)
                {
                    char *lines[] = {
                        "Would you like to update now?"};

                    enum rrc_prompt_result result = rrc_prompt_yes_no(xfb, lines, 1);
                    if (result == RRC_PROMPT_RESULT_YES)
                    {
                        rrc_update_do_updates();
                    }
                }
                else if (option->label == exit_label)
                {
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

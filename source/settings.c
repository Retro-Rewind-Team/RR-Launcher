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
#include <stdio.h>
#include <string.h>
#include <gctypes.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

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
    /** An array of possible selectable option names, if this is a select option. */
    char **options;
};

// NOTE: all option arrays must have a NULL at the end, and the length (including the NULL) must be >= 2 (so there should be at least one option).
static char *my_stuff_options[] = {"Disabled", "Retro Rewind", "CTGP", NULL};
static char *enabled_disabled_options[] = {"Disabled", "Enabled", NULL};
static char *language_options[] = {"English", "Japanese", "French", "German", NULL};
static char *launch_label = "Launch";
static char *save_label = "Save changes";
static char *exit_label = "Exit";

enum rrc_settings_result rrc_settings_display()
{
    rrc_con_clear(true);

    struct settings_option options[] = {
        {.type = OPTION_TYPE_BUTTON, .label = launch_label},
        {.type = OPTION_TYPE_SELECT, .label = "My Stuff", .options = my_stuff_options, .margin_top = 1},
        {.type = OPTION_TYPE_SELECT, .label = "Language", .options = language_options},
        {.type = OPTION_TYPE_SELECT, .label = "Separate savegame", .options = enabled_disabled_options},
        {.type = OPTION_TYPE_BUTTON, .label = save_label, .margin_top = 1},
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

        if (options[i].type == OPTION_TYPE_SELECT && options[i].options[0] == NULL)
        {
            RRC_FATAL("'%s' is a select option but contains an empty options array", options[i].label);
        }

        int len = strlen(options[i].label);
        if (len > max_label_len)
        {
            max_label_len = len;
        }
    }

    while (1)
    {
        int row = _RRC_SPLASH_ROW + 5;
        for (int i = 0; i < option_count; i++)
        {
            const struct settings_option *option = &options[i];

            // add any extra "newlines" (which means just seek)
            row += option->margin_top;
            rrc_con_cursor_seek_to(row, 0);
            printf(RRC_CON_ANSI_CLEAR_LINE);

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
                return RRC_SETTINGS_EXIT;
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
                    // pressed left even though we're at the first option already
                    // "wrap" back to the last option (the one just before the `NULL` element)
                    // we don't store the length, so loop there
                    while (option->options[option->selected_option + 1] != NULL)
                        option->selected_option++;
                }
                break;
            }

            if ((pressed & RRC_WPAD_RIGHT_MASK) && option->type == OPTION_TYPE_SELECT)
            {
                option->selected_option++;
                // if we're now at a `NULL` (which every option array has at the end)
                // it means the user pressed right while already at the last option,
                // in which case we wrap back to the first one
                if (option->options[option->selected_option] == NULL)
                {
                    option->selected_option = 0;
                }
                break;
            }

            if (pressed & RRC_WPAD_A_MASK)
            {
                if (option->label == launch_label)
                {
                    return RRC_SETTINGS_LAUNCH;
                }
                else if (option->label == exit_label)
                {
                    return RRC_SETTINGS_EXIT;
                }
            }

            usleep(RRC_WPAD_LOOP_TIMEOUT);
        }
    }
    return RRC_SETTINGS_LAUNCH;
}

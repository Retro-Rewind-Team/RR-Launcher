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

static char *my_stuff_options[] = {"Disabled", "Retro Rewind", "CTGP", NULL};
static char *enabled_disabled_options[] = {"Disabled", "Enabled", NULL};
static char *language_options[] = {"English", "Japanese", "French", "German", NULL};
static char *launch_label = "Launch";
static char *exit_label = "Exit";

enum rrc_settings_result rrc_settings_display()
{
    rrc_con_clear(true);

    struct settings_option options[] = {
        {.type = OPTION_TYPE_BUTTON, .label = launch_label},
        {.type = OPTION_TYPE_SELECT, .label = "My Stuff", .options = my_stuff_options, .margin_top = 1},
        {.type = OPTION_TYPE_SELECT, .label = "Language", .options = language_options},
        {.type = OPTION_TYPE_SELECT, .label = "Separate savegame", .options = enabled_disabled_options},
        {.type = OPTION_TYPE_BUTTON, .label = exit_label, .margin_top = 1},
    };
    const int option_count = sizeof(options) / sizeof(struct settings_option);
    int selected_idx = 0;

    // used for padding the label string with spaces so that all options are aligned with each other
    u32 max_label_len = 0;
    for (int i = 0; i < option_count; i++)
    {
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

            if ((pressed & RRC_WPAD_DOWN_MASK) && selected_idx < option_count - 1)
            {
                selected_idx++;
                break;
            }

            if ((pressed & RRC_WPAD_UP_MASK) && selected_idx > 0)
            {
                selected_idx--;
                break;
            }

            struct settings_option *option = &options[selected_idx];

            if ((pressed & RRC_WPAD_LEFT_MASK) && option->type == OPTION_TYPE_SELECT && option->selected_option > 0)
            {
                option->selected_option--;
                break;
            }

            if ((pressed & RRC_WPAD_RIGHT_MASK) && option->type == OPTION_TYPE_SELECT && option->options[option->selected_option + 1] != NULL)
            {
                option->selected_option++;
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

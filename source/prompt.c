/*
    update.h - UI prompt implementation
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include <ogc/wiilaunch.h>

#include "console.h"
#include "prompt.h"
#include "util.h"
#include "gui.h"

#define _RRC_OPTIONS_W_PAD "         " /* 9 spaces between each option */
#define _RRC_PROMPT_TEXT_FIRST_ROW 7
#define _RRC_PROMPT_OPTIONS_PAD 1
#define _RRC_PROMPT_LINES_MAX 10

static void *prompt_xfb = NULL;

void _rrc_prompt_alloc_xfb()
{
    GXRModeObj *rmode = rrc_gui_get_video_mode();
    prompt_xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    CON_Init(prompt_xfb, 0, 0, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
}

void _rrc_prompt_reinit_xfb()
{
    GXRModeObj *rmode = rrc_gui_get_video_mode();
    CON_Init(prompt_xfb, 0, 0, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
}

void _rrc_prompt_xfb_setup()
{
    if (prompt_xfb == NULL)
    {
        _rrc_prompt_alloc_xfb();
    }
    else
    {
        _rrc_prompt_reinit_xfb();
    }
}

enum rrc_prompt_result rrc_prompt_2_options(
    void *old_xfb,
    char **lines,
    int n,
    char *option1,
    char *option2,
    enum rrc_prompt_result option1_result,
    enum rrc_prompt_result option2_result)
{
    _rrc_prompt_xfb_setup();

    if (n >= _RRC_PROMPT_LINES_MAX)
    {
        rrc_gui_display_con(old_xfb, false);
        return RRC_PROMPT_RESULT_ERROR;
    }

    rrc_gui_display_con(prompt_xfb, true);
    rrc_gui_display_banner(prompt_xfb);

    rrc_con_display_splash();

    int cols, rows;
    CON_GetMetrics(&cols, &rows);

    for (int i = 0; i < n; i++)
    {
        if (strlen(lines[i]) > cols)
        {
            rrc_gui_display_con(old_xfb, false);
            return RRC_PROMPT_RESULT_ERROR;
        }

        rrc_con_print_text_centered(_RRC_PROMPT_TEXT_FIRST_ROW + i, lines[i]);
    }

    /* initialize all buttons stuff */
    int buttons_line = _RRC_PROMPT_TEXT_FIRST_ROW + n + _RRC_PROMPT_OPTIONS_PAD;
    char *arrow = RRC_CON_ANSI_FG_BRIGHT_WHITE ">> " RRC_CON_ANSI_FG_WHITE;
    int rendered_arrow_len = 3;
    char buttons[cols];
    snprintf(buttons, cols, "%s%s%s", option1, _RRC_OPTIONS_W_PAD, option2);

    if ((strlen(buttons) + strlen(arrow)) > cols)
    {
        rrc_gui_display_con(old_xfb, false);
        return RRC_PROMPT_RESULT_ERROR;
    }
    int buttons_col = rrc_con_centered_text_start_column(buttons);

    rrc_con_print_text_centered(buttons_line, buttons);

    enum rrc_prompt_result selected_option = option1_result;
    enum rrc_prompt_result prev_selected_option = -1;

    int dir_pressed = 0;

    while (1)
    {
        PAD_ScanPads();
        WPAD_ScanPads();
        int wiipressed = WPAD_ButtonsDown(0);
        int gcpressed = PAD_ButtonsDown(0);

        if ((wiipressed & (WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT)) || (gcpressed & (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT)))
        {
            dir_pressed = 1;
            selected_option = (selected_option == option1_result ? option2_result : option1_result);
        }
        else if (dir_pressed && (!(wiipressed & (RRC_WPAD_LEFT_MASK | RRC_WPAD_RIGHT_MASK)) || !(gcpressed & (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT))))
        {
            dir_pressed = 0;
        }
        else if (wiipressed & RRC_WPAD_A_MASK || gcpressed & PAD_BUTTON_A)
        {
            break;
        }

        if (selected_option == option2_result && prev_selected_option != selected_option)
        {
            rrc_con_print_text_centered(buttons_line, buttons);
            rrc_con_cursor_seek_to(buttons_line, buttons_col + strlen(buttons) - rendered_arrow_len - strlen(option2));
            printf(arrow);
        }
        else if (prev_selected_option != selected_option)
        {
            rrc_con_print_text_centered(buttons_line, buttons);
            rrc_con_cursor_seek_to(buttons_line, buttons_col - rendered_arrow_len);
            printf(arrow);
        }

        prev_selected_option = selected_option;

        usleep(RRC_WPAD_LOOP_TIMEOUT);
    }

    rrc_gui_display_con(old_xfb, false);
    return selected_option;
}

enum rrc_prompt_result rrc_prompt_yes_no(void *old_xfb, char **lines, int n)
{
    return rrc_prompt_2_options(old_xfb, lines, n, "Yes", "No", RRC_PROMPT_RESULT_YES, RRC_PROMPT_RESULT_NO);
}

enum rrc_prompt_result rrc_prompt_ok_cancel(void *old_xfb, char **lines, int n)
{
    return rrc_prompt_2_options(old_xfb, lines, n, "OK", "Cancel", RRC_PROMPT_RESULT_OK, RRC_PROMPT_RESULT_CANCEL);
}

void rrc_prompt_1_option(void *old_xfb,
                         char **lines,
                         int n,
                         char *button)
{
    _rrc_prompt_xfb_setup();

    if (n >= _RRC_PROMPT_LINES_MAX)
    {
        rrc_gui_display_con(old_xfb, false);
        return;
    }

    rrc_gui_display_con(prompt_xfb, true);
    rrc_gui_display_banner(prompt_xfb);

    rrc_con_display_splash();

    int cols, rows;
    CON_GetMetrics(&cols, &rows);

    for (int i = 0; i < n; i++)
    {
        if (strlen(lines[i]) > cols)
        {
            rrc_gui_display_con(old_xfb, false);
            return;
        }

        rrc_con_print_text_centered(_RRC_PROMPT_TEXT_FIRST_ROW + i, lines[i]);
    }

    char *arrow = RRC_CON_ANSI_FG_BRIGHT_WHITE ">> " RRC_CON_ANSI_FG_WHITE;
    int rendered_arrow_len = 3;

    int buttons_line = _RRC_PROMPT_TEXT_FIRST_ROW + n + _RRC_PROMPT_OPTIONS_PAD;
    int buttons_col = rrc_con_centered_text_start_column(button);

    rrc_con_print_text_centered(buttons_line, button);
    rrc_con_cursor_seek_to(buttons_line, buttons_col - rendered_arrow_len);
    printf(arrow);

    // just wait for an A press lol
    while (1)
    {
        WPAD_ScanPads();
        PAD_ScanPads();
        int wiipressed = WPAD_ButtonsDown(0);
        int gcpressed = PAD_ButtonsDown(0);
        if (wiipressed & RRC_WPAD_A_MASK || gcpressed & PAD_BUTTON_A)
        {
            break;
        }

        usleep(RRC_WPAD_LOOP_TIMEOUT);
    }

    rrc_gui_display_con(old_xfb, false);
    return;
}

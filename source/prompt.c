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

#define _RRC_PROMPT_TEXT_FIRST_ROW 7
#define _RRC_PROMPT_OPTIONS_PAD 1
#define _RRC_PROMPT_LINES_MAX 10

static void *prompt_xfb = NULL;

void _rrc_prompt_alloc_xfb()
{
    GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
    prompt_xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(prompt_xfb, 0, 0, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    rrc_con_set_line_width_chars(rmode->fbWidth / (sizeof(char) * 8 /* bits */));
}

void _rrc_prompt_reinit_xfb()
{
    GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
    console_init(prompt_xfb, 0, 0, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    rrc_con_set_line_width_chars(rmode->fbWidth / (sizeof(char) * 8 /* bits */));
}

void _rrc_prompt_upd_framebuffer(void *xfb)
{
    /*
        This is a giga hack, re-init'ing the console clears xfb so we save it and restore it,
        and then flush stdout and reset back to gray foreground.
    */
    GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
    int fbsize = VIDEO_GetFrameBufferSize(rmode);
    char *saved = malloc(fbsize);
    memcpy(saved, xfb, fbsize);

    console_init(xfb, 0, 0, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    memcpy(xfb, saved, fbsize);
    free(saved);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    printf(RRC_CON_ANSI_FG_WHITE);
    fflush(stdout);
}

enum rrc_prompt_result rrc_prompt_yes_no(void *old_xfb, char **lines, int n)
{
    if (prompt_xfb == NULL)
    {
        _rrc_prompt_alloc_xfb();
    }
    else
    {
        _rrc_prompt_reinit_xfb();
    }

    if (n >= _RRC_PROMPT_LINES_MAX)
    {
        goto err;
    }

    _rrc_prompt_upd_framebuffer(prompt_xfb);

    rrc_con_display_splash();

    for (int i = 0; i < n; i++)
    {
        if (strlen(lines[i]) > rrc_con_get_line_width_chars())
        {
            goto err;
        }

        rrc_con_print_text_centered(_RRC_PROMPT_TEXT_FIRST_ROW + i, lines[i]);
    }

    int buttons_line = _RRC_PROMPT_TEXT_FIRST_ROW + n + _RRC_PROMPT_OPTIONS_PAD;
    char *buttons = "Yes         No";
    char *arrow = ">> ";
    int buttons_col = rrc_con_centered_text_start_column(buttons);

    rrc_con_print_text_centered(buttons_line, buttons);

    enum rrc_prompt_result selected_option = RRC_PROMPT_RESULT_NO;
    enum rrc_prompt_result prev_selected_option = -1;

    int dir_pressed = 0;

    while (1)
    {
        WPAD_ScanPads();
        int pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_LEFT || pressed & WPAD_BUTTON_RIGHT)
        {
            dir_pressed = 1;
            selected_option = !selected_option;
        }
        else if (dir_pressed && !(pressed & RRC_WPAD_LEFT_MASK) && !(pressed & RRC_WPAD_RIGHT_MASK))
        {
            dir_pressed = 0;
        }
        else if (pressed & RRC_WPAD_A_MASK)
        {
            break;
        }

        if (selected_option == RRC_PROMPT_RESULT_NO && prev_selected_option != selected_option)
        {
            rrc_con_print_text_centered(buttons_line, buttons);
            rrc_con_cursor_seek_to(buttons_line, buttons_col + strlen(buttons) - strlen(arrow) - 2);
            printf(arrow);
        }
        else if (prev_selected_option != selected_option)
        {
            rrc_con_print_text_centered(buttons_line, buttons);
            rrc_con_cursor_seek_to(buttons_line, buttons_col - strlen(arrow));
            printf(arrow);
        }

        prev_selected_option = selected_option;
    }

    _rrc_prompt_upd_framebuffer(old_xfb);
    return selected_option;

err:
    _rrc_prompt_upd_framebuffer(old_xfb);
    return RRC_PROMPT_RESULT_ERROR;
}
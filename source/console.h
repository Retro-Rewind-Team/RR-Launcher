/*
    console.h - console helper routines header file

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

#ifndef RRC_CONSOLE_H
#define RRC_CONSOLE_H

#include <stdbool.h>

#define _RRC_SPLASH "RETRO REWIND"
#define _RRC_SPLASH_ROW 3

#define _RRC_PROGRESS_ROW 6

#define _RRC_ACTION_ROW 9

#define _RRC_PRINTF_ROW 15

#define RRC_CON_ANSI_FG_BLACK "\x1b[30;0m"
#define RRC_CON_ANSI_FG_RED "\x1b[31;0m"
#define RRC_CON_ANSI_FG_GREEN "\x1b[32;0m"
#define RRC_CON_ANSI_FG_YELLOW "\x1b[33;0m"
#define RRC_CON_ANSI_FG_BLUE "\x1b[34;0m"
#define RRC_CON_ANSI_FG_MAGENTA "\x1b[35;0m"
#define RRC_CON_ANSI_FG_CYAN "\x1b[36;0m"
#define RRC_CON_ANSI_FG_WHITE "\x1b[37;0m"
#define RRC_CON_ANSI_FG_BRIGHT_BLACK "\x1b[30;1m"
#define RRC_CON_ANSI_FG_BRIGHT_RED "\x1b[31;1m"
#define RRC_CON_ANSI_FG_BRIGHT_GREEN "\x1b[32;1m"
#define RRC_CON_ANSI_FG_BRIGHT_YELLOW "\x1b[33;1m"
#define RRC_CON_ANSI_FG_BRIGHT_BLUE "\x1b[34;1m"
#define RRC_CON_ANSI_FG_BRIGHT_MAGENTA "\x1b[35;1m"
#define RRC_CON_ANSI_FG_BRIGHT_CYAN "\x1b[36;1m"
#define RRC_CON_ANSI_FG_BRIGHT_WHITE "\x1b[37;1m"

#define RRC_CON_ANSI_BG_BLACK "\x1b[40;0m"
#define RRC_CON_ANSI_BG_RED "\x1b[41;0m"
#define RRC_CON_ANSI_BG_GREEN "\x1b[42;0m"
#define RRC_CON_ANSI_BG_YELLOW "\x1b[43;0m"
#define RRC_CON_ANSI_BG_BLUE "\x1b[44;0m"
#define RRC_CON_ANSI_BG_MAGENTA "\x1b[45;0m"
#define RRC_CON_ANSI_BG_CYAN "\x1b[46;0m"
#define RRC_CON_ANSI_BG_WHITE "\x1b[47;0m"
#define RRC_CON_ANSI_BG_BRIGHT_BLACK "\x1b[40;1m"
#define RRC_CON_ANSI_BG_BRIGHT_RED "\x1b[41;1m"
#define RRC_CON_ANSI_BG_BRIGHT_GREEN "\x1b[42;1m"
#define RRC_CON_ANSI_BG_BRIGHT_YELLOW "\x1b[43;1m"
#define RRC_CON_ANSI_BG_BRIGHT_BLUE "\x1b[44;1m"
#define RRC_CON_ANSI_BG_BRIGHT_MAGENTA "\x1b[45;1m"
#define RRC_CON_ANSI_BG_BRIGHT_CYAN "\x1b[46;1m"
#define RRC_CON_ANSI_BG_BRIGHT_WHITE "\x1b[47;1m"

#define RRC_CON_ANSI_CLEAR_LINE "\x1b[0K"
#define RRC_CON_ANSI_CLEAR_SCREEN "\x1b[2J"

/* reset to black backgroumd, white foreground */
#define RRC_CON_ANSI_CLR  \
    RRC_CON_ANSI_FG_WHITE \
    RRC_CON_ANSI_BG_BLACK

#define RRC_CON_EDGE_PAD 2

void rrc_con_set_action(char *action);

void rrc_con_set_progress_percent(int progress);

void rrc_con_update(char *action, int progress_percent);

void rrc_con_set_line_width_chars(int chars);

int rrc_con_get_line_width_chars();

void rrc_con_cursor_seek_to(int row, int column);

void rrc_con_cursor_seek_to_row_centered(int row, int text_len);

void rrc_con_print_text_centered(int row, char *text);

/*
    Get the column at which centered text would start printing from.
*/
int rrc_con_centered_text_start_column(char *text);

void rrc_con_display_splash();

void rrc_con_display_progress_bar();

void rrc_con_display_action();

void rrc_con_print_state();

void rrc_con_clear(bool keep_splash);

#endif

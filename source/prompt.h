/*
    update.h - UI prompt headers
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

enum rrc_prompt_result
{
    /* Problem with input parameters, usually */
    RRC_PROMPT_RESULT_ERROR = -1,
    RRC_PROMPT_RESULT_YES = 0,
    RRC_PROMPT_RESULT_NO = 1
};

/*
    Creates a user prompt. All `lines' are printed on the screen in order, centered,
    and below the user is presented with either `yes' or `no' to select.
    Each line has a newline appended, you do not need to append them yourself.
    `lines' is limited to 10 entries. Each line cannot exceed the console line width.
    `n' contains the amount of lines in `lines'.

    This function returns RRC_PROMPT_RESULT_YES if `yes' is selected and RRC_PROMPT_RESULT_NO if `no' is selected.
    On error, RRC_PROMPT_RESULT_ERROR is returned.
*/
enum rrc_prompt_result rrc_prompt_yes_no(void *old_xfb, char **lines, int n);
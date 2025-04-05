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

#ifndef RRC_PROMPT_H
#define RRC_PROMPT_H

enum rrc_prompt_result
{
    /* Problem with input parameters, usually */
    RRC_PROMPT_RESULT_ERROR = -1,
    RRC_PROMPT_RESULT_YES = 0,
    RRC_PROMPT_RESULT_NO = 1,
    RRC_PROMPT_RESULT_OK = 2,
    RRC_PROMPT_RESULT_CANCEL = 3
};

/*
    See `rrc_prompt_2_options' for a description of prompts.

    This function returns RRC_PROMPT_RESULT_YES if `Yes' is selected and RRC_PROMPT_RESULT_NO if `No' is selected.
    On error, RRC_PROMPT_RESULT_ERROR is returned.
*/
enum rrc_prompt_result rrc_prompt_yes_no(void *old_xfb, char **lines, int n);

/*
    See `rrc_prompt_2_options' for a description of prompts.

    This function returns RRC_PROMPT_RESULT_OK if `OK' is selected and RRC_PROMPT_RESULT_CANCEL if `Cancel' is selected.
    On error, RRC_PROMPT_RESULT_ERROR is returned.
*/
enum rrc_prompt_result rrc_prompt_ok_cancel(void *old_xfb, char **lines, int n);

/**
    Creates a user prompt. All `lines' are printed on the screen in order, centered,
    and below the user is presented with either `yes' or `no' to select.
    Each line has a newline appended, you do not need to append them yourself.
    `lines' is limited to 10 entries. Each line cannot exceed the console line width.
    `n' contains the amount of lines in `lines'.
    `option1' and `option2' are the available buttons to display. `option1_result' and `option2_result'
    are the values that the buttons map to.

    This function returns option1_result or option2_result depending on which option is selected.
    On error, RRC_PROMPT_RESULT_ERROR is returned.
 */
enum rrc_prompt_result rrc_prompt_2_options(
    void *old_xfb,
    char **lines,
    int n,
    char *option1,
    char *option2,
    enum rrc_prompt_result option1_result,
    enum rrc_prompt_result option2_result);

#endif

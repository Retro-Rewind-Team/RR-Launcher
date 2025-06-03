/*
    pad.h - Unified gamepad/wiimote interface

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

#include <gctypes.h>
#include <wiiuse/wpad.h>
#include <ogc/pad.h>

struct pad_state
{
    u32 wpad;
    u32 gc;
};

#define RRC_WPAD_A_MASK (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)
#define RRC_WPAD_B_MASK (WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B)
#define RRC_WPAD_HOME_MASK (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)
#define RRC_WPAD_PLUS_MASK (WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS)
#define RRC_WPAD_UP_MASK (WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP)
#define RRC_WPAD_RIGHT_MASK (WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT)
#define RRC_WPAD_DOWN_MASK (WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN)
#define RRC_WPAD_LEFT_MASK (WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT)
#define RRC_WPAD_LEFT_RIGHT_MASK (RRC_WPAD_LEFT_MASK | RRC_WPAD_RIGHT_MASK)

#define RRC_PAD_LEFT_RIGHT_MASK (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT)

/**
 * Reads button states for all controllers (Wiimote, CC, GCN).
 */
struct pad_state rrc_pad_buttons();

inline bool rrc_pad_a_pressed(struct pad_state state)
{
    return (state.wpad & RRC_WPAD_A_MASK) || (state.gc & PAD_BUTTON_A);
}

inline bool rrc_pad_b_pressed(struct pad_state state)
{
    return (state.wpad & RRC_WPAD_B_MASK) || (state.gc & PAD_BUTTON_B);
}

/**
 * Checks if the home button is pressed (Wiimote & CC), or the start button (GC).
 */
inline bool rrc_pad_home_pressed(struct pad_state state)
{
    return (state.wpad & RRC_WPAD_HOME_MASK) || (state.gc & PAD_BUTTON_START);
}

/**
 * Checks if the left or right buttons are pressed.
 */
inline bool rrc_pad_left_right_pressed(struct pad_state state)
{
    return (state.wpad & RRC_WPAD_LEFT_RIGHT_MASK) || (state.gc & RRC_PAD_LEFT_RIGHT_MASK);
}

inline bool rrc_pad_up_pressed(struct pad_state state)
{
    return (state.wpad & RRC_WPAD_UP_MASK) || (state.gc & PAD_BUTTON_UP);
}

inline bool rrc_pad_down_pressed(struct pad_state state)
{
    return (state.wpad & RRC_WPAD_DOWN_MASK) || (state.gc & PAD_BUTTON_DOWN);
}

inline bool rrc_pad_left_pressed(struct pad_state state)
{
    return (state.wpad & RRC_WPAD_LEFT_MASK) || (state.gc & PAD_BUTTON_LEFT);
}

inline bool rrc_pad_right_pressed(struct pad_state state)
{
    return (state.wpad & RRC_WPAD_RIGHT_MASK) || (state.gc & PAD_BUTTON_RIGHT);
}

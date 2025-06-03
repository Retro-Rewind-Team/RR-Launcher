/*
    pad.h - Unified gamepad/wiimote implementation

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

#include "pad.h"
#include <gctypes.h>
#include <wiiuse/wpad.h>
#include <ogc/pad.h>

struct pad_state rrc_pad_buttons()
{
    WPAD_ScanPads();
    PAD_ScanPads();

    u32 wpad = WPAD_ButtonsDown(0);
    u32 gc = PAD_ButtonsDown(0);

    struct pad_state state = {.wpad = wpad, .gc = gc};
    return state;
}

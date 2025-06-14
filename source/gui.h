/*
    gui.h - headers for initialisation and layout of gui

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

    `rrc_gui_video_fix' uses code adapted from Brainslug:
    Copyright (C) 2014, Alex Chadwick

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef RRC_GUI_H
#define RRC_GUI_H

#include <gccore.h>

/*
    Initialises the main GUI.

    The GUI is comprised of two parts: a console, and a banner artwork which surrounds it.
    This routine initialises and displays both.

    Note that this routine should only ever be called once: subsequent draws of either
    component should be done using other available routines.

    The created framebuffer is returned in `xfb'. Set `sys_stdio_report' to `true' to
    enable stdio reporting; this can be used, for example, to redirect printf calls to
    Dolphin logs.
*/
void rrc_gui_xfb_alloc(void **xfb, bool sys_stdio_report);

/*
    Initialises a console in the provided `xfb', and optionally clears it.

    Console position and dimensions are decided based on the video aspect ratio and resolution.
*/
void rrc_gui_display_con(void *xfb, bool clear_console);

/*
    Displays the GUI banner appropriate to the current aspect ratio within the provided `xfb'.

    The `xfb' is assumed to fill the entire VI!

    Returns 0 status code n success, -1 on error.
*/
int rrc_gui_display_banner(void *xfb);

/*
    Using the preferred video mode seems to not work for some configurations.

    Define our own subset of supported resolutions.
*/
GXRModeObj* rrc_gui_get_video_mode();

/*
    Set a video mode that will load properly.

    This code is part of Brainslug, adapted for this channel.
    See copyright notice at the start of this file.

    https://github.com/Chadderz121/brainslug-wii/blob/8ca49384452dcb7d41e90d002ba0f85b4e57bf57/src/apploader/apploader.c#L114
*/
void rrc_gui_video_fix(char region);

#endif
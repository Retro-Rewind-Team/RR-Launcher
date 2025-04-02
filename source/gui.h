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
*/

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
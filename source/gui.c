/*
    gui.c - implementation for initialisation and layout of gui

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

#include <stdlib.h>
#include <string.h>

#include "gui.h"
#include "pngu/pngu.h"
#include "util.h"

#define _RRC_GUI_CON_START_X_4_3 70
#define _RRC_GUI_CON_START_Y_4_3 114
#define _RRC_GUI_CON_WIDTH_4_3 502
#define _RRC_GUI_CON_HEIGHT_4_3 300

void rrc_gui_xfb_alloc(void **xfb, bool sys_stdio_report)
{
    // Initialise the video system
    VIDEO_Init();

    // Obtain the preferred video mode from the system
    // This will correspond to the settings in the Wii menu
    GXRModeObj *rmode = rrc_gui_get_video_mode();
    // Allocate memory for the display in the uncached region
    *xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    SYS_STDIO_Report(sys_stdio_report);

    VIDEO_SetBlack(true);
}

void rrc_gui_display_con(void *xfb, bool clear_console)
{
    GXRModeObj *rmode = rrc_gui_get_video_mode();

    // Set up the video registers with the chosen mode
    VIDEO_Configure(rmode);
    // Tell the video hardware where our display memory is
    VIDEO_SetNextFramebuffer(xfb);
    // Make the display visible
    VIDEO_SetBlack(false);
    if (clear_console)
        VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
    // Flush the video register changes to the hardware
    VIDEO_Flush();
    // Wait for Video setup to complete
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    /*
        CON_Init always clears the part of xfb being used by the console
        so we have to save xfb, init the console, and then restore xfb
        if we don't want to clear the console
    */
    if (!clear_console)
    {
        int fbsize = VIDEO_GetFrameBufferSize(rmode);
        char *saved = malloc(fbsize);
        memcpy(saved, xfb, fbsize);

        CON_Init(
            xfb,
            _RRC_GUI_CON_START_X_4_3,
            _RRC_GUI_CON_START_Y_4_3,
            _RRC_GUI_CON_WIDTH_4_3,
            _RRC_GUI_CON_HEIGHT_4_3,
            rmode->fbWidth * VI_DISPLAY_PIX_SZ);

        memcpy(xfb, saved, fbsize);
        free(saved);
    }
    else
    {
        CON_Init(
            xfb,
            _RRC_GUI_CON_START_X_4_3,
            _RRC_GUI_CON_START_Y_4_3,
            _RRC_GUI_CON_WIDTH_4_3,
            _RRC_GUI_CON_HEIGHT_4_3,
            rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    }
}

int _rrc_gui_draw_banner(void *xfb, char *banner, GXRModeObj *rmode)
{
    IMGCTX ctx = NULL;
    PNGUPROP imgProp;
    s32 ret = -1;

    /* Select PNG data */
    ctx = PNGU_SelectImageFromBuffer(banner);
    if (!ctx)
    {
        ret = -1;
        goto out;
    }

    /* Get image properties */
    ret = PNGU_GetImageProperties(ctx, &imgProp);
    if (ret != PNGU_OK)
    {
        ret = -1;
        goto out;
    }

    PNGU_DECODE_TO_COORDS_YCbYCr(ctx, 0, 0, imgProp.imgWidth, imgProp.imgHeight, rmode->fbWidth, rmode->xfbHeight, xfb);

    ret = 0;

out:
    /* Free memory */
    if (ctx)
        PNGU_ReleaseImageContext(ctx);

    return ret;
}

int rrc_gui_display_banner(void *xfb)
{
    GXRModeObj *rmode = rrc_gui_get_video_mode();
    extern char banner4_3[];
    return _rrc_gui_draw_banner(xfb, banner4_3, rmode);
}

/* set video mode in appropriate memory map value */
void _rrc_gui_set_mm_video_mode(int mode)
{
    *((u32 *)0x800000CC) = mode;
    rrc_invalidate_cache((void *)0x800000CC, 1);
}

GXRModeObj *rrc_gui_get_video_mode()
{
    return VIDEO_GetPreferredMode(NULL);
}

/*
    This code is part of Brainslug, adapted for this channel.
    See copyright notice at the start of this file.

    https://github.com/Chadderz121/brainslug-wii/blob/8ca49384452dcb7d41e90d002ba0f85b4e57bf57/src/apploader/apploader.c#L114
*/
void rrc_gui_video_fix(char region)
{
    /* Get video mode configuration */
    bool progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();
    bool PAL60 = CONF_GetEuRGB60() > 0;
    u32 tvmode = CONF_GetVideo();

    int r_rmode_reg = 0;
    void *r_rmode = VIDEO_GetPreferredMode(0);

    switch (tvmode)
    {
    case CONF_VIDEO_PAL:
        r_rmode_reg = PAL60 ? VI_EURGB60 : VI_PAL;
        r_rmode = progressive ? &TVEurgb60Hz480Prog : (PAL60 ? &TVEurgb60Hz480IntDf : &TVPal528IntDf);
        break;

    case CONF_VIDEO_MPAL:
        r_rmode_reg = VI_MPAL;
        r_rmode = progressive ? &TVEurgb60Hz480Prog : &TVMpal480IntDf;
        break;

    case CONF_VIDEO_NTSC:
        r_rmode_reg = VI_NTSC;
        r_rmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
        break;
    }

    switch (region)
    {
    case 'D':
    case 'F':
    case 'P':
    case 'X':
    case 'Y':
        r_rmode_reg = PAL60 ? VI_EURGB60 : VI_PAL;
        r_rmode = progressive ? &TVEurgb60Hz480Prog : (PAL60 ? &TVEurgb60Hz480IntDf : &TVPal528IntDf);
        break;
    case 'E':
    case 'J':
        r_rmode_reg = VI_NTSC;
        r_rmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
    }

    (*(volatile unsigned int *)0x800000cc) = r_rmode_reg;
    rrc_invalidate_cache((void *)0x800000cc, 4);

    if (r_rmode != 0)
    {
        VIDEO_Configure(r_rmode);
    }
}
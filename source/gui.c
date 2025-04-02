/*
    gui.h - implementation for initialisation and layout of gui

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

#include <gccore.h>
#include <stdlib.h>
#include <string.h>

#include "pngu/pngu.h"
#include "util.h"

#define _RRC_GUI_CON_START_X_4_3 70
#define _RRC_GUI_CON_START_Y_4_3 114
#define _RRC_GUI_CON_WIDTH_4_3 570
#define _RRC_GUI_CON_HEIGHT_4_3 420

enum _rrc_gui_aspect_ratio
{
    _RRC_GUI_ASP_UNKNOWN,
    _RRC_GUI_ASP_4_3,
    _RRC_GUI_ASP_16_9,
};

enum _rrc_gui_aspect_ratio _rrc_gui_get_aspect_ratio(GXRModeObj *rmode)
{
    int out = (rmode->viWidth * 100) / rmode->viHeight;

    /*
        TODO: the Wii only supports anamorphic widescreen,
        so in order to properly support it, we need to
        detect if it is enabled using SYSCONF.

        If it is detected, extend rmode->viWidth to 720 then set
        the configuration. From there we can load the correct banner.

        This also means this function is pointless - SYSCONF will
        effectively tell us the aspect ratio, so in future, remove
        this and instead use SYSCONF along with viWidth to set
        widescreen mode properly.

        See https://forum.wiibrew.org/read.php?11,74268
    */

    /* 4:3 */
    if (out == (4 * 100) / 3)
    {
        return _RRC_GUI_ASP_4_3;
    }
    else if (out == (16 * 100) / 9)
    {
        return _RRC_GUI_ASP_16_9;
    }

    return _RRC_GUI_ASP_UNKNOWN;
}

void rrc_gui_xfb_alloc(void **xfb, bool sys_stdio_report)
{
    // Initialise the video system
    VIDEO_Init();

    // Obtain the preferred video mode from the system
    // This will correspond to the settings in the Wii menu
    GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
    // Allocate memory for the display in the uncached region
    *xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    SYS_STDIO_Report(sys_stdio_report);

    VIDEO_SetBlack(true);
}

void rrc_gui_display_con(void *xfb, bool clear_console)
{
    GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);

    // Set up the video registers with the chosen mode
    VIDEO_Configure(rmode);
    // Tell the video hardware where our display memory is
    VIDEO_SetNextFramebuffer(xfb);
    // Make the display visible
    VIDEO_SetBlack(false);
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
    GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);

    extern char banner4_3[];
    extern char banner16_9[];

    enum _rrc_gui_aspect_ratio asp = _rrc_gui_get_aspect_ratio(rmode);

    if (asp == _RRC_GUI_ASP_16_9)
    {
        return _rrc_gui_draw_banner(xfb, banner16_9, rmode);
    }
    else if (asp == _RRC_GUI_ASP_4_3)
    {
        return _rrc_gui_draw_banner(xfb, banner4_3, rmode);
    }
    else
    {
        RRC_FATAL("unknown aspect ratio/resolution: %ix%i", rmode->viWidth, rmode->viHeight);
    }
}
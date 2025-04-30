/*
    sd.h - SD card helper and initialisation routine headers

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

#ifndef RRC_SD_H
#define RRC_SD_H

#include "result.h"

#define RRC_SD_TEST_FILE ".sdtest"

/*
    Initialises and tests the SD card slot.

    If the SD card is inserted and unlocked, this function returns successfully.
    Otherwise, an error is returned. This error can either be treated as fatal or
    prompt the user to retry inserting with an unlocked SD card.
*/
struct rrc_result rrc_sd_init();

#endif
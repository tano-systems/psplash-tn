/*
 *  pslash - a lightweight framebuffer splashscreen for embedded devices.
 *
 *  Copyright (c) 2020 Tano Systems LLC
 *  Author: Anton Kikin <a.kikin@tano-systems.com>
 *
 *  Copyright (c) 2014 MenloSystems GmbH
 *  Author: Olaf Mandel <o.mandel@menlosystems.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef _HAVE_PSPLASH_CONFIG_H
#define _HAVE_PSPLASH_CONFIG_H

/* Text to output on program start; if undefined, output nothing */
#define PSPLASH_STARTUP_MSG ""

/* Bool indicating if the image is fullscreen, as opposed to split screen */
#define PSPLASH_IMG_FULLSCREEN 1

/* Position of the image split from top edge, numerator of fraction */
#define PSPLASH_IMG_SPLIT_NUMERATOR 5

/* Position of the image split from top edge, denominator of fraction */
#define PSPLASH_IMG_SPLIT_DENOMINATOR 6

/* Padding for message (space in pixels between message and progress bar) */
#define PSPLASH_MSG_PADDING 4

/* Progress bar width (in pixels) */
#define PSPLASH_BAR_WIDTH 340

/* Progress bar height (in pixels) */
#define PSPLASH_BAR_HEIGHT 30

/* Progress bar border thickness (in pixels) */
#define PSPLASH_BAR_BORDER_WIDTH 2

/* Size in pixels between progress bar and its border */
#define PSPLASH_BAR_BORDER_SPACE 2

#if defined(ENABLE_ALIVE_GIF)

/* Position of the alive image split from top edge, numerator of fraction */
#define PSPLASH_ALIVE_IMG_V_SPLIT_NUMERATOR 1

/* Position of the alive image split from top edge, denominator of fraction */
#define PSPLASH_ALIVE_IMG_V_SPLIT_DENOMINATOR 6

/* Position of the alive image split from left edge, numerator of fraction */
#define PSPLASH_ALIVE_IMG_H_SPLIT_NUMERATOR 1

/* Position of the alive image split from left edge, denominator of fraction */
#define PSPLASH_ALIVE_IMG_H_SPLIT_DENOMINATOR 6

#endif

#endif

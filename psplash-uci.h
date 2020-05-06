/*
 *  pslash - a lightweight framebuffer splashscreen for embedded devices.
 *
 *  Copyright (c) 2020 Tano Systems LLC, Anton Kikin <a.kikin@tano-systems.com>
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

#ifndef _HAVE_PSPLASH_UCI_H
#define _HAVE_PSPLASH_UCI_H

#include <uci.h>
#include "psplash-fb.h"

#define PSPLASH_STARTUP_MSG_SIZE 256
#define PSPLASH_IMAGE_PATH_SIZE 256

typedef struct PSplashConfig
{
	char startup_msg[PSPLASH_STARTUP_MSG_SIZE];

	char image[PSPLASH_IMAGE_PATH_SIZE];

	int disable_console_switch;
	int angle;
	int fbdev_id;

	int img_fullscreen;
	int img_split_numerator;
	int img_split_denominator;

	int msg_padding;
	int bar_width;
	int bar_height;
	int bar_border_width;
	int bar_border_space;

	struct
	{
		PSplashColor background;
		PSplashColor text;
		PSplashColor bar;
		PSplashColor bar_background;
		PSplashColor bar_border;
		PSplashColor bar_border_space;

	} colors;

} PSplashConfig;

extern PSplashConfig config;

int psplash_uci_read_config(void);

#endif

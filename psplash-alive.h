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

#ifndef _HAVE_PSPLASH_ALIVE_H
#define _HAVE_PSPLASH_ALIVE_H

#include "psplash-fb.h"

enum PSplashAliveAnimationMode
{
	PSPLASH_ALIVE_ANIMATION_MODE_AUTO = 0, /* Automatically */
	PSPLASH_ALIVE_ANIMATION_MODE_MSG = 1,  /* By 'ALIVE' message */
};

#if defined(ENABLE_ALIVE_GIF)

int psplash_alive_load(void);
int psplash_alive_init(void);
unsigned int psplash_alive_frame_duration_ms(void);
int psplash_alive_frame_render(PSplashFB *fb);
void psplash_alive_frame_next(void);
int psplash_alive_destroy(void);

#else

static inline int psplash_alive_load(void)
{
	return 0;
}

static inline int psplash_alive_init(void)
{
	return 0;
}

static unsigned int psplash_alive_frame_duration_ms(void)
{
	return 0;
}

static inline int psplash_alive_frame_render(PSplashFB *fb)
{
	return 0;
}

static inline void psplash_alive_frame_next(void)
{
}

static inline int psplash_alive_destroy(void)
{
	return 0;
}

#endif

#endif

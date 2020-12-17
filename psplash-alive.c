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

#include "psplash.h"
#include "psplash-fb.h"
#include "psplash-uci.h"
#include "psplash-config.h"

#if defined(ENABLE_ALIVE_GIF)

#include "gifdec.h"

struct frame
{
	uint8_t *data;
	unsigned int delay_ms;
	struct frame *next;
};

static struct frame *frames = NULL;
static struct frame *frames_head = NULL;

static gd_GIF *gif = NULL;

int psplash_alive_load(void)
{
	if (!config.alive.enable)
		return 0;

	gif = gd_open_gif(config.alive.image);
	if (!gif) {
		ulog(LOG_ERR, "alive: Can't open file \"%s\"\n", config.alive.image);
		return -1;
	}

#if DEBUG
	ulog(LOG_DEBUG, "alive: Loaded GIF from file \"%s\" (width = %d, height = %d)\n",
		config.alive.image, gif->width, gif->height);
#endif

	return 0;
}

int psplash_alive_init(void)
{
	int frames_num = 0;

	if (!gif)
		return 0;

	while (gd_get_frame(gif))
	{
		struct frame *f;

		++frames_num;

		f = calloc(1, sizeof(struct frame));
		if (!f)
		{
			ulog(LOG_ERR,
				"alive: Failed to allocate memory for frame #%d structure\n",
				frames_num);
			return -1;
		}

		/*
		 * 24-bit RGB values:
		 *   [0] = R (8 bit)
		 *   [1] = G (8 bit)
		 *   [2] = B (8 bit)
		 */
		f->data = calloc(1, 3 * gif->width * gif->height);
		if (!f->data)
		{
			ulog(LOG_ERR,
				"alive: Failed to allocate memory for frame #%d data\n",
				frames_num);
			free(f);
			return -1;
		}

		/* GCE delay is in hundreths of a second */
		f->delay_ms = gif->gce.delay * 10;

#if DEBUG
		ulog(LOG_DEBUG, "alive: Frame #%d: duration %u ms\n", frames_num, f->delay_ms);
#endif

		f->next = frames_head;

		gd_render_frame(gif, f->data);

		if (frames)
			frames->next = f;
		else
			frames_head = f;

		frames = f;
	}

#if DEBUG
	ulog(LOG_DEBUG, "alive: Readed %d frame(s)\n", frames_num);
#endif

	return 0;
}

unsigned int psplash_alive_frame_duration_ms(void)
{
	if (!frames_head)
		return 0;

	return frames_head->delay_ms;
}

static void psplash_alive_draw_frame(
	PSplashFB *fb,
	uint8_t *data,
	int width,
	int height,
	int x,
	int y
)
{
	int dx;
	int dy;

	uint8_t *color = data;

	for (dy = 0; dy < height; dy++)
	{
		for (dx = 0; dx < width; dx++)
		{
			if (!gd_is_bgcolor(gif, color))
			{
				psplash_fb_plot_pixel(fb, x + dx, y + dy,
					psplash_color_make(color[0], color[1], color[2]));
			}

			color += 3;
		}
	}
}

int psplash_alive_frame_render(PSplashFB *fb)
{
	if (!frames_head)
		return 0;

	psplash_alive_draw_frame(
		fb,
		frames_head->data,
		gif->width,
		gif->height,
		fb->width * config.alive.img_h_split_numerator /
			config.alive.img_h_split_denominator - gif->width / 2,
		fb->height * config.alive.img_v_split_numerator /
			config.alive.img_v_split_denominator - gif->height / 2
	);

	return 0;
}

void psplash_alive_frame_next(void)
{
	/* Next frame */
	if (frames_head)
		frames_head = frames_head->next;
}

int psplash_alive_destroy(void)
{
	if (frames_head)
	{
		struct frame *f = frames_head;

		do
		{
			struct frame *next = f->next;

			if (f->data)
				free(f->data);

			free(f);

			f = next;

		} while(f != frames_head);
	}

	if (gif)
		gd_close_gif(gif);

	return 0;
}

#endif

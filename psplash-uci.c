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
#include "psplash-config.h"
#include "psplash-colors.h"

static PSplashConfig default_config =
{
	.angle                  = 0,
	.fbdev_id               = 0,
	.disable_console_switch = 0,
	.ignore_msg_cmds        = 0,
	.enable_bar             = 1,
	.enable_msg             = 1,
	.startup_msg            = PSPLASH_STARTUP_MSG,
	.img_fullscreen         = PSPLASH_IMG_FULLSCREEN,
	.img_split_numerator    = PSPLASH_IMG_SPLIT_NUMERATOR,
	.img_split_denominator  = PSPLASH_IMG_SPLIT_DENOMINATOR,
	.msg_padding            = PSPLASH_MSG_PADDING,
	.bar_width              = PSPLASH_BAR_WIDTH,
	.bar_height             = PSPLASH_BAR_HEIGHT,
	.bar_border_width       = PSPLASH_BAR_BORDER_WIDTH,
	.bar_border_space       = PSPLASH_BAR_BORDER_SPACE,
	.colors =
	{
		.background       = { PSPLASH_BACKGROUND_COLOR },
		.text             = { PSPLASH_TEXT_COLOR },
		.bar              = { PSPLASH_BAR_COLOR },
		.bar_background   = { PSPLASH_BAR_BACKGROUND_COLOR },
		.bar_border       = { PSPLASH_BAR_BORDER_COLOR },
		.bar_border_space = { PSPLASH_BAR_BORDER_SPACE_COLOR }
	},

#if defined(ENABLE_ALIVE_GIF)
	.alive =
	{
		.enable                  = 0,
		.animation_mode          = PSPLASH_ALIVE_ANIMATION_MODE_AUTO,
		.img_v_split_numerator   = PSPLASH_ALIVE_IMG_V_SPLIT_NUMERATOR,
		.img_v_split_denominator = PSPLASH_ALIVE_IMG_V_SPLIT_DENOMINATOR,
		.img_h_split_numerator   = PSPLASH_ALIVE_IMG_H_SPLIT_NUMERATOR,
		.img_h_split_denominator = PSPLASH_ALIVE_IMG_H_SPLIT_DENOMINATOR
	},
#endif
};

PSplashConfig config;

static inline PSplashColor ul2color(unsigned long ul)
{
	PSplashColor color =
	{
		.red   = (ul & 0xff0000) >> 16,
		.green = (ul & 0xff00) >> 8,
		.blue  = (ul & 0xff),
	};

	return color;
}

int psplash_uci_read_config(void)
{
	struct uci_context *uci_ctx = uci_alloc_context();
	struct uci_package *p;
	struct uci_ptr ptr = { .package = "psplash" };

	if (uci_load(uci_ctx, "psplash", &p) != UCI_OK)
	{
		uci_free_context(uci_ctx);
		return -1;
	}

	/* Set default configuration */
	memcpy(&config, &default_config, sizeof(PSplashConfig));

	/*
	 * Configuration
	 */
	ptr.s = NULL;
	ptr.section = "config";

	/* config.image */
	ptr.o = NULL;
	ptr.option = "image";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
	{
		strncpy(config.image, ptr.o->v.string,
			sizeof(config.image) - 1);
	}

	/* config.startup_msg */
	ptr.o = NULL;
	ptr.option = "startup_msg";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
	{
		strncpy(config.startup_msg, ptr.o->v.string,
			sizeof(config.startup_msg) - 1);
	}

	/* config.ignore_msg_cmds */
	ptr.o = NULL;
	ptr.option = "ignore_msg_cmds";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.ignore_msg_cmds = !!(int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.enable_bar */
	ptr.o = NULL;
	ptr.option = "enable_bar";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.enable_bar = !!(int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.enable_msg */
	ptr.o = NULL;
	ptr.option = "enable_msg";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.enable_msg = !!(int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.disable_console_switch */
	ptr.o = NULL;
	ptr.option = "disable_console_switch";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.disable_console_switch = !!(int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.angle */
	ptr.o = NULL;
	ptr.option = "angle";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.angle = (int)strtoul(ptr.o->v.string, NULL, 0);

	if ((config.angle != 0) &&
	    (config.angle != 90) &&
	    (config.angle != 180) &&
	    (config.angle != 270))
	{
		ulog(LOG_ERR, "uci: Readed unsupported config.angle = %d\n", config.angle);
		config.angle = 0;
	}

	/* config.fbdev_id */
	ptr.o = NULL;
	ptr.option = "fbdev_id";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.fbdev_id = (int)strtoul(ptr.o->v.string, NULL, 0);

	if ((config.fbdev_id < 0) ||
	    (config.fbdev_id > 9))
	{
		ulog(LOG_ERR, "uci: Readed unsupported config.fbdev_id = %d\n", config.fbdev_id);
		config.fbdev_id = 0;
	}

	/* config.img_fullscreen */
	ptr.o = NULL;
	ptr.option = "img_fullscreen";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.img_fullscreen = !!(int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.img_split_numerator */
	ptr.o = NULL;
	ptr.option = "img_split_numerator";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.img_split_numerator = (int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.img_split_denominator */
	ptr.o = NULL;
	ptr.option = "img_split_denominator";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.img_split_denominator = (int)strtoul(ptr.o->v.string, NULL, 0);

	if (!config.img_split_denominator)
	{
		ulog(LOG_ERR, "uci: Readed unsupported config.img_split_denominator = %d (fallback to 1)\n",
			config.img_split_denominator);
		config.img_split_denominator = 1;
	}

	/* config.msg_padding */
	ptr.o = NULL;
	ptr.option = "msg_padding";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.msg_padding = (int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.bar_width */
	ptr.o = NULL;
	ptr.option = "bar_width";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.bar_width = (int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.bar_height */
	ptr.o = NULL;
	ptr.option = "bar_height";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.bar_height = (int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.bar_border_width */
	ptr.o = NULL;
	ptr.option = "bar_border_width";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.bar_border_width = (int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.bar_border_space */
	ptr.o = NULL;
	ptr.option = "bar_border_space";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.bar_border_space = (int)strtoul(ptr.o->v.string, NULL, 0);

	/*
	 * Colors
	 */
	ptr.s = NULL;
	ptr.section = "colors";

	/* colors.background */
	ptr.o = NULL;
	ptr.option = "background";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.colors.background = ul2color(strtoul(ptr.o->v.string, NULL, 16));

	/* colors.text */
	ptr.o = NULL;
	ptr.option = "text";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.colors.text = ul2color(strtoul(ptr.o->v.string, NULL, 16));

	/* colors.bar */
	ptr.o = NULL;
	ptr.option = "bar";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.colors.bar = ul2color(strtoul(ptr.o->v.string, NULL, 16));

	/* colors.bar_background */
	ptr.o = NULL;
	ptr.option = "bar_background";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.colors.bar_background = ul2color(strtoul(ptr.o->v.string, NULL, 16));

	/* colors.bar_border */
	ptr.o = NULL;
	ptr.option = "bar_border";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.colors.bar_border = ul2color(strtoul(ptr.o->v.string, NULL, 16));

	/* colors.bar_border_space */
	ptr.o = NULL;
	ptr.option = "bar_border_space";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.colors.bar_border_space = ul2color(strtoul(ptr.o->v.string, NULL, 16));

#if defined(ENABLE_ALIVE_GIF)
	/*
	 * Alive image/messages configuration
	 */
	ptr.s = NULL;
	ptr.section = "alive";

	/* enable */
	ptr.o = NULL;
	ptr.option = "enable";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.alive.enable = !!(int)strtoul(ptr.o->v.string, NULL, 0);

	/* image path */
	ptr.o = NULL;
	ptr.option = "image";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
	{
		strncpy(config.alive.image, ptr.o->v.string,
			sizeof(config.alive.image) - 1);
	}

	/* animation mode */
	ptr.o = NULL;
	ptr.option = "animation_mode";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
	{
		if (!strcmp(ptr.o->v.string, "message") ||
		    !strcmp(ptr.o->v.string, "msg"))
			config.alive.animation_mode = PSPLASH_ALIVE_ANIMATION_MODE_MSG;
		else if (!strcmp(ptr.o->v.string, "auto"))
			config.alive.animation_mode = PSPLASH_ALIVE_ANIMATION_MODE_AUTO;
		else
		{
			ulog(LOG_ERR, "uci: Unknown config.alive.animation_mode '%s' (fallback to 'auto')\n",
				ptr.o->v.string);
			config.alive.animation_mode = PSPLASH_ALIVE_ANIMATION_MODE_AUTO;
		}
	}

	/* config.alive.img_v_split_numerator */
	ptr.o = NULL;
	ptr.option = "img_v_split_numerator";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.alive.img_v_split_numerator = (int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.alive.img_v_split_denominator */
	ptr.o = NULL;
	ptr.option = "img_v_split_denominator";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.alive.img_v_split_denominator = (int)strtoul(ptr.o->v.string, NULL, 0);

	if (!config.alive.img_v_split_denominator)
	{
		ulog(LOG_ERR, "uci: Readed unsupported config.alive.img_v_split_denominator = %d (fallback to 1)\n",
			config.alive.img_v_split_denominator);
		config.alive.img_v_split_denominator = 1;
	}

	/* config.alive.img_h_split_numerator */
	ptr.o = NULL;
	ptr.option = "img_h_split_numerator";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.alive.img_h_split_numerator = (int)strtoul(ptr.o->v.string, NULL, 0);

	/* config.alive.img_h_split_denominator */
	ptr.o = NULL;
	ptr.option = "img_h_split_denominator";

	uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ptr.o && (ptr.o->type == UCI_TYPE_STRING))
		config.alive.img_h_split_denominator = (int)strtoul(ptr.o->v.string, NULL, 0);

	if (!config.alive.img_h_split_denominator)
	{
		ulog(LOG_ERR, "uci: Readed unsupported config.alive.img_h_split_denominator = %d (fallback to 1)\n",
			config.alive.img_h_split_denominator);
		config.alive.img_h_split_denominator = 1;
	}
#endif

	uci_free_context(uci_ctx);
	return 0;
}

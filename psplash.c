/*
 *  pslash - a lightweight framebuffer splashscreen for embedded devices.
 *
 *  Copyright (c) 2006 Matthew Allum <mallum@o-hand.com>
 *
 *  Parts of this file ( fifo handling ) based on 'usplash' copyright
 *  Matthew Garret.
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
#include "font.h"

#define SPLIT_LINE_POS(fb)                                                         \
	((fb)->height - ((config.img_split_denominator - config.img_split_numerator) * \
					 (fb)->height / config.img_split_denominator))

void psplash_exit(int UNUSED(signum))
{
	ulog(LOG_DEBUG, "mark\n");

	psplash_console_reset();
}

void psplash_draw_msg(PSplashFB *fb, const char *msg)
{
	int w, h;

	psplash_fb_text_size(&w, &h, &font, msg);

	ulog(LOG_DEBUG, "displaying '%s' %ix%i\n", msg ? msg : "(null)", w, h);

	/* Clear */

	psplash_fb_draw_rect(
		fb,
		0,
		SPLIT_LINE_POS(fb) - h - config.msg_padding,
		fb->width,
		h,
		config.colors.background);

	if (msg)
	{
		psplash_fb_draw_text(
			fb,
			(fb->width - w) / 2,
			SPLIT_LINE_POS(fb) - h - config.msg_padding,
			config.colors.text,
			&font,
			msg);
	}
}

void psplash_draw_progress(PSplashFB *fb, int value)
{
	int x, y, width, height, barwidth;
	int padding = config.bar_border_width + config.bar_border_space;

	x = ((fb->width - config.bar_width) / 2) + padding;
	y = SPLIT_LINE_POS(fb) + padding;

	width  = config.bar_width - padding * 2;
	height = config.bar_height - padding * 2;

	if (value > 0)
	{
		barwidth = (CLAMP(value, 0, 100) * width) / 100;
		psplash_fb_draw_rect(
			fb, x + barwidth, y, width - barwidth, height, config.colors.bar_background);
		psplash_fb_draw_rect(fb, x, y, barwidth, height, config.colors.bar);
	}
	else
	{
		barwidth = (CLAMP(-value, 0, 100) * width) / 100;
		psplash_fb_draw_rect(fb, x, y, width - barwidth, height, config.colors.bar_background);
		psplash_fb_draw_rect(fb, x + width - barwidth, y, barwidth, height, config.colors.bar);
	}

	ulog(LOG_DEBUG, "value: %i, width: %i, barwidth :%i\n", value, width, barwidth);
}

void psplash_draw_progress_border(PSplashFB *fb)
{
	int x = (fb->width - config.bar_width) / 2;
	int y = SPLIT_LINE_POS(fb);

	if (config.bar_border_width > 0)
	{
		/* border */
		psplash_fb_draw_box(
			fb,
			x,
			y,
			config.bar_width,
			config.bar_height,
			config.bar_border_width,
			config.colors.bar_border);

		if (config.bar_border_space > 0)
		{
			/* border space */
			psplash_fb_draw_box(
				fb,
				x + config.bar_border_width,
				y + config.bar_border_width,
				config.bar_width - config.bar_border_width * 2,
				config.bar_height - config.bar_border_width * 2,
				config.bar_border_space,
				config.colors.bar_border_space);
		}
	}
}

static int parse_single_command(PSplashFB *fb, char *string)
{
	char *command;

	ulog(LOG_DEBUG, "got cmd %s\n", string);

	if (strcmp(string, "QUIT") == 0)
		return 1;

	command = strtok(string, " ");

	if (!strcmp(command, "PROGRESS"))
	{
		psplash_draw_progress(fb, atoi(strtok(NULL, "\0")));
	}
	else if (!strcmp(command, "MSG"))
	{
		if (!config.ignore_msg_cmds)
			psplash_draw_msg(fb, strtok(NULL, "\0"));
	}
	else if (!strcmp(command, "QUIT"))
	{
		return 1;
	}

	psplash_fb_flip(fb, 0);
	return 0;
}

static int parse_commands(PSplashFB *fb, char *string, int len)
{
	int   i;
	char *command = string;

	for (i = 0; i < len; i++)
	{
		if ((string[i] == 0) || (string[i] == '\n'))
		{
			string[i] = 0;
			if (parse_single_command(fb, command))
				return 1;

			command = &string[i + 1];
		}
	}

	return 0;
}

void psplash_main(PSplashFB *fb, int pipe_fd, int timeout)
{
	int            err;
	ssize_t        length = 0;
	fd_set         descriptors;
	struct timeval tv;
	char *         end;
	char           command[2048];

	tv.tv_sec  = timeout;
	tv.tv_usec = 0;

	FD_ZERO(&descriptors);
	FD_SET(pipe_fd, &descriptors);

	end = command;

	while (1)
	{
		if (timeout != 0)
			err = select(pipe_fd + 1, &descriptors, NULL, NULL, &tv);
		else
			err = select(pipe_fd + 1, &descriptors, NULL, NULL, NULL);

		if (err <= 0)
		{
			/*
			if (errno == EINTR)
			  continue;
			*/
			return;
		}

		length += read(pipe_fd, end, sizeof(command) - (end - command));

		if (length == 0)
		{
			/* Reopen to see if there's anything more for us */
			close(pipe_fd);
			pipe_fd = open(PSPLASH_FIFO, O_RDONLY | O_NONBLOCK);
			goto out;
		}

		if (command[length - 1] == '\0')
		{
			if (parse_commands(fb, command, length))
				return;
			length = 0;
		}
		else if (command[length - 1] == '\n')
		{
			command[length - 1] = '\0';
			if (parse_commands(fb, command, length))
				return;
			length = 0;
		}

	out:
		end = &command[length];

		tv.tv_sec  = timeout;
		tv.tv_usec = 0;

		FD_ZERO(&descriptors);
		FD_SET(pipe_fd, &descriptors);
	}

	return;
}

int main(int argc, char **argv)
{
	PSplashImage image;
	char *     tmpdir;
	int        pipe_fd, i = 0, ret = 0;
	PSplashFB *fb;

	ulog_open(ULOG_STDIO, LOG_DAEMON, "psplash");
#if DEBUG
	ulog_threshold(LOG_DEBUG);
#else
	ulog_threshold(LOG_INFO);
#endif

	if (psplash_uci_read_config())
	{
		perror("uci config");
		exit(-1);
	}

	signal(SIGHUP, psplash_exit);
	signal(SIGINT, psplash_exit);
	signal(SIGQUIT, psplash_exit);

	while (++i < argc)
	{
		if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--no-console-switch"))
		{
			config.disable_console_switch = TRUE;
			continue;
		}

		if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--angle"))
		{
			if (++i >= argc)
				goto fail;
			config.angle = atoi(argv[i]);
			continue;
		}

		if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--fbdev"))
		{
			if (++i >= argc)
				goto fail;
			config.fbdev_id = atoi(argv[i]);
			continue;
		}

	fail:
		fprintf(
			stderr,
			"Usage: %s [-n|--no-console-switch][-a|--angle <0|90|180|270>][-f|--fbdev <0..9>]\n",
			argv[0]);
		exit(-1);
	}

	if (psplash_image_read(&image, config.image))
		exit(-1);

	tmpdir = getenv("TMPDIR");

	if (!tmpdir)
		tmpdir = "/tmp";

	chdir(tmpdir);

	if (mkfifo(PSPLASH_FIFO, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))
	{
		if (errno != EEXIST)
		{
			perror("mkfifo");
			exit(-1);
		}
	}

	pipe_fd = open(PSPLASH_FIFO, O_RDONLY | O_NONBLOCK);

	if (pipe_fd == -1)
	{
		perror("pipe open");
		exit(-2);
	}

	if (!config.disable_console_switch)
		psplash_console_switch();

	if ((fb = psplash_fb_new(config.angle, config.fbdev_id)) == NULL)
	{
		ret = -1;
		goto fb_fail;
	}

	/* Clear the background with background color */
	psplash_fb_draw_rect(fb, 0, 0, fb->width, fb->height, config.colors.background);

	/* Draw the Poky logo  */
	if (config.img_fullscreen)
	{
		psplash_fb_draw_img(
			&image,
			fb,
			(fb->width - image.width) / 2,
			(fb->height - image.height) / 2
		);
	}
	else
	{
		psplash_fb_draw_img(
			&image,
			fb,
			(fb->width - image.width) / 2,
			(fb->height * config.img_split_numerator / config.img_split_denominator -
				image.height) / 2
		);
	}

	/* Draw progress bar border */
	psplash_draw_progress_border(fb);

	/* Draw initial progress bar */
	psplash_draw_progress(fb, 0);

	if (strlen(config.startup_msg))
		psplash_draw_msg(fb, config.startup_msg);

	psplash_image_free(&image);

	/* Scene set so let's flip the buffers. */
	/* The first time we also synchronize the buffers so we can build on an
	 * existing scene. After the first scene is set in both buffers, only the
	 * text and progress bar change which overwrite the specific areas with every
	 * update.
	*/
	psplash_fb_flip(fb, 1);

	psplash_main(fb, pipe_fd, 0);

	psplash_fb_destroy(fb);

fb_fail:
	unlink(PSPLASH_FIFO);

	if (!config.disable_console_switch)
		psplash_console_reset();

	return ret;
}

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

#ifdef ENABLE_PNG
#include <png.h>
#endif

#ifdef ENABLE_JPEG
#include <jpeglib.h>
#endif

#include "psplash.h"
#include "psplash-image.h"

#ifdef ENABLE_JPEG

static int psplash_image_read_jpeg(PSplashImage *image, const char *filename)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *infile;
	int y;
	JSAMPARRAY buffer;
	int row_stride;

	if ((infile = fopen(filename, "rb")) == NULL)
	{
		ulog(LOG_ERR, "jpeg: Can't open file \"%s\"\n", filename);
		return -1;
	}

	ulog(LOG_DEBUG, "jpeg: Reading from \"%s\"\n", filename);

	/* Step 1: allocate and initialize JPEG decompression object */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */
	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */
	(void)jpeg_read_header(&cinfo, TRUE);

	image->width = cinfo.image_width;
	image->height = cinfo.image_height;

	ulog(LOG_DEBUG, "jpeg: size = %d x %d\n", image->width, image->height);

	/* Step 4: set parameters for decompression */
	cinfo.out_color_space = JCS_EXT_RGBA;

	/* Step 5: Start decompressor */
	(void)jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;

	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	ulog(LOG_DEBUG, "jpeg: output_components = %d\n", cinfo.output_components);

	image->row_pointers =
		(uint8 **)malloc(sizeof(uint8 *) * image->height);

	if (!image->row_pointers)
	{
		ulog(LOG_ERR, "jpeg: Failed to allocate memory for row pointers\n");
		return -1;
	}

	for (y = 0; y < image->height; y++)
	{
		image->row_pointers[y] =
			(uint8 *)malloc(row_stride);

		if (!image->row_pointers[y])
		{
			ulog(LOG_ERR, "jpeg: Failed to allocate memory for row %d\n", y);
			return -1;
		}
	}

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
	while(cinfo.output_scanline < image->height)
	{
		jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(image->row_pointers[cinfo.output_scanline - 1], buffer[0], row_stride);
	}

	/* Step 7: Finish decompression */
	(void)jpeg_finish_decompress(&cinfo);

	/* Step 8: Release JPEG decompression object */
	jpeg_destroy_decompress(&cinfo);

	fclose(infile);
	jpeg_destroy_decompress(&cinfo);

	return 0;
}

#endif

#ifdef ENABLE_PNG

static int psplash_image_read_png(PSplashImage *image, const char *filename)
{
	int y;
	png_structp png = NULL;
	png_infop info = NULL;
	png_byte color_type;
	png_byte bit_depth;

	FILE *fp;

	fp = fopen(filename, "rb");
	if (!fp)
	{
		ulog(LOG_ERR, "png: Can't open file \"%s\"\n", filename);
		return -1;
	}

	ulog(LOG_DEBUG, "png: Reading from \"%s\"\n", filename);

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png)
	{
		ulog(LOG_ERR, "png: png_create_read_struct() failed\n");
		return -1;
	}

	info = png_create_info_struct(png);
	if(!info)
	{
		ulog(LOG_ERR, "png: png_create_info_struct() failed\n");
		return -1;
	}

	if (setjmp(png_jmpbuf(png)))
	{
		fclose(fp);
		png_destroy_read_struct(&png, &info, NULL);
		return -1;
	}

	png_init_io(png, fp);

	png_read_info(png, info);

	image->width  = png_get_image_width(png, info);
	image->height = png_get_image_height(png, info);
	color_type    = png_get_color_type(png, info);
	bit_depth     = png_get_bit_depth(png, info);

	ulog(LOG_DEBUG, "png: size = %d x %d\n", image->width, image->height);
	ulog(LOG_DEBUG, "png: color_type = %d\n", color_type);
	ulog(LOG_DEBUG, "png: bit_depth = %d\n", bit_depth);

	/* Read any color_type into 8bit depth, RGBA format.
	 * See http://www.libpng.org/pub/png/libpng-manual.txt */

	if (bit_depth == 16)
		png_set_strip_16(png);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	/* PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth. */
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if (png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	/* These color_type don't have an alpha channel then fill it with 0xff. */
	if (color_type == PNG_COLOR_TYPE_RGB ||
	    color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xff, PNG_FILLER_AFTER);

	if (color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);

	image->row_pointers =
		(png_bytep *)malloc(sizeof(png_bytep) * image->height);

	if (!image->row_pointers)
	{
		ulog(LOG_ERR, "png: Failed to allocate memory for row pointers\n");
		return -1;
	}

	for (y = 0; y < image->height; y++)
	{
		image->row_pointers[y] =
			(png_byte *)malloc(png_get_rowbytes(png, info));

		if (!image->row_pointers[y])
		{
			ulog(LOG_ERR, "png: Failed to allocate memory for row %d\n", y);
			return -1;
		}
	}

	png_read_image(png, image->row_pointers);

	ulog(LOG_DEBUG, "png: Successfully readed\n");

	fclose(fp);
	png_destroy_read_struct(&png, &info, NULL);

	return 0;
}

#endif

int psplash_image_read(PSplashImage *image, const char *filename)
{
	const char *ext = strrchr(filename, '.');

	if (ext)
	{
		ext = ext + 1;

#ifdef ENABLE_PNG
		if (!strcmp(ext, "png"))
			return psplash_image_read_png(image, filename);
#endif

#ifdef ENABLE_JPEG
		if (!strcmp(ext, "jpg") || !strcmp(ext, "jpeg"))
			return psplash_image_read_jpeg(image, filename);
#endif
	}

	ulog(LOG_ERR, "Unsupported image format (%s)\n",
		ext ? ext : "no extension");

	return -1;
}

void psplash_image_free(PSplashImage *image)
{
	int y;

	if (image->row_pointers)
	{
		for (y = 0; y < image->height; y++)
			free(image->row_pointers[y]);

		free(image->row_pointers);
	}

	memset(image, 0, sizeof(PSplashImage));

	ulog(LOG_DEBUG, "png: Freed memory\n");
}

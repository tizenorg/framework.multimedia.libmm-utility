/*
 * libmm-utility
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: YoungHun Kim <yh8004.kim@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <glib/gstdio.h>

#include <scmn_base.h>
/**
 *  Image file format
 */
typedef enum
{
    /* Image file format */
    MM_UTILITY_IMAGE_FILE_FMT_NONE,			/**< Nothing or not supported image file format */
    MM_UTILITY_IMAGE_FILE_FMT_BMP,  			/**< Windows bitmap : .bmp, .dib, .rle, .2bp */
    MM_UTILITY_IMAGE_FILE_FMT_GIF,         		/**< Graphics Interchange Format : .gif, .gfa, .giff  */
    MM_UTILITY_IMAGE_FILE_FMT_PNG, 	        	/**< Portable Network Graphics : .png */
    MM_UTILITY_IMAGE_FILE_FMT_JPG,         		/**< Joint Photographic Experts Group : .jpg, .jpeg, .jpe */
} mm_utility_image_file_format;



/**
 * YUV format for gif
 */
typedef enum
{
    /* YUV planar format */
    MM_UTILITY_GIF_FMT_YUV420 = 0x00,  /**< YUV420 format - planer */
    MM_UTILITY_GIF_FMT_YUV422,         /**< YUV422 format - planer */
    /* YUV packed format */
    MM_UTILITY_GIF_FMT_UYVY,            /**< UYVY format - YUV packed format */
} mm_utility_gif_yuv_format;

/**
 * YUV data for gif
 */
typedef struct 
{
    mm_utility_gif_yuv_format format;     /**< pixel format*/
    int width;                          /**< width */
    int height;                         /**< heigt */
    int size;                           /**< size */
    void *data;                         /**< data */
} mm_utility_decoded_data;

static int get_file_format ( const char * input_file )
{
	mm_utility_image_file_format file_format = MM_UTILITY_IMAGE_FILE_FMT_NONE;

	if ( g_str_has_suffix( input_file, "bmp" )
		|| g_str_has_suffix ( input_file, "dib" )
		|| g_str_has_suffix ( input_file, "rle" )
		|| g_str_has_suffix ( input_file, "2bp" )
		|| g_str_has_suffix( input_file, "BMP" )
		|| g_str_has_suffix ( input_file, "DIB" )
		|| g_str_has_suffix ( input_file, "RLE" )
		|| g_str_has_suffix ( input_file, "2BP" ) )
	{
		printf ( "Image file format is BMP. \n" );
		file_format = MM_UTILITY_IMAGE_FILE_FMT_BMP;
	}
	else if ( g_str_has_suffix ( input_file, "gif" )
		|| g_str_has_suffix ( input_file, "gfa" )
		|| g_str_has_suffix ( input_file, "giff" )
		|| g_str_has_suffix ( input_file, "GIF" )
		|| g_str_has_suffix ( input_file, "GFA" )
		|| g_str_has_suffix ( input_file, "GIFF" ))
	{
		printf ( "Image file format is GIF. \n" );
		file_format = MM_UTILITY_IMAGE_FILE_FMT_GIF;
	}
	else if ( g_str_has_suffix ( input_file, "png" )
		||  g_str_has_suffix ( input_file, "PNG" ))
	{
		printf ( "Image file format is PNG. \n" );
		file_format = MM_UTILITY_IMAGE_FILE_FMT_PNG;
	}
	else if ( g_str_has_suffix ( input_file, "jpg" )
		|| g_str_has_suffix ( input_file, "jpeg" )
		|| g_str_has_suffix ( input_file, "jpe" )
		|| g_str_has_suffix ( input_file, "JPG" )
		|| g_str_has_suffix ( input_file, "JPEG" )
		|| g_str_has_suffix ( input_file, "JPE" ) )
	{
		printf ( "Image file format is JPG. \n" );
		file_format = MM_UTILITY_IMAGE_FILE_FMT_JPG;
	}
	else
	{
		printf ( "Not supported image file format. \n" );
	}

	return file_format;

}

static int  malloc_decoded_picture ( SCMN_IMGB * img )
{
	int ret = 0;
	int malloc_size = 0;

	switch ( img->cs )	{
		case SCMN_CS_YUV444 :
			printf ("colorspace is YUV444.\n");
			img->w[1] = img->w[2] = img->w[0];
			img->h[1] = img->h[2] = img->h[0];
			img->s[0] = img->s[1] = img->s[2] = img->w[0];
			img->e[0] = img->e[1] = img->e[2] = img->h[0];

			malloc_size = img->s[0]*img->e[0]*sizeof(unsigned char) * 3;

			break;

		case SCMN_CS_YUV422 :
			printf ("colorspace is YUV422 or YUV422N.\n");

			img->w[1] = img->w[2] = (img->w[0]+1) >> 1;
			img->h[1] = img->h[2] = img->h[0];
			img->s[0] = img->w[0];
			img->s[1] = img->s[2] = (img->w[0]+1) >> 1;
			img->e[0] = img->e[1] = img->e[2] = img->h[0];

			malloc_size = img->s[0]*img->e[0]*sizeof(unsigned char) * 3 >> 1;

			break;

		case SCMN_CS_YUV422W :
			printf ("colorspace is YUV422W.\n");

			img->w[1] = img->w[2] = img->w[0];
			img->h[1] = img->h[2] = (img->h[0]+1) >> 1;
			img->s[0] = img->s[1] = img->s[2] = img->w[0];
			img->e[0] = img->h[0];
			img->e[1] = img->e[2] = (img->h[0]+1) >> 1;

			malloc_size = img->s[0]*img->e[0]*sizeof(unsigned char) * 3 >> 1;

			break;

		case SCMN_CS_YUV420 :
			printf ("colorspace is YUV420.\n");

			/* plus 1 for rounding */
			img->w[1] = img->w[2] = (img->w[0]+1) >> 1;
			img->h[1] = img->h[2] = (img->h[0]+1) >> 1;
			img->s[0] = img->w[0];
			img->s[1] = img->s[2] = (img->w[0]+1)  >> 1;
			img->e[0] = img->h[0];
			img->e[1] = img->e[2] = (img->h[0]+1) >> 1;

			malloc_size = img->s[0]*img->e[0]*sizeof(unsigned char) * 3 >> 1;

			break;

		case SCMN_CS_YUV400 :
			printf ("colorspace is YUV400.\n");

			img->s[0] = img->w[0];
			img->e[0] = img->h[0];

			malloc_size = img->s[0]*img->e[0]*sizeof(unsigned char);

			break;

		case SCMN_CS_RGB565 :
			printf ("colorspace is RGB565.\n");

			img->s[0] = img->w[0];
			img->e[0] = img->h[0];

			malloc_size = img->s[0]*img->e[0]*sizeof(unsigned char) * 3 >> 1;

			break;

		case SCMN_CS_RGB888 :
			printf ("colorspace is RGB888.\n");

			img->s[0] = img->w[0]*3;
			img->e[0] = img->h[0];

			printf ( "%d x %d \n", img->s[0], img->e[0] );
			malloc_size = img->s[0]*img->e[0]*sizeof(unsigned char) * 3 ;

			break;

		case SCMN_CS_RGBA8888 :
			printf ("colorspace is RGBA8888.\n");

			img->s[0] = img->w[0];
			img->e[0] = img->h[0];

			malloc_size = img->s[0]*img->e[0]*sizeof(unsigned char) * 4 ;

			break;

		default :
			printf ( "Not supported colorspace[%d].\n", img->cs );
			ret = -1;
			break;
	}

	/* allocate buffer */
	if ( SCMN_CS_IS_YUV ( img->cs ) )
	{
		if(img->cs == SCMN_CS_YUV400) {
			img->a[0] = malloc(img->s[0]*img->e[0]*sizeof(unsigned char));
		} else {
			img->a[0] = malloc(img->s[0]*img->e[0]*sizeof(unsigned char));
			img->a[1] = malloc(img->s[1]*img->e[1]*sizeof(unsigned char));
			img->a[2] = malloc(img->s[2]*img->e[2]*sizeof(unsigned char));
		}
	}
	else if ( SCMN_CS_IS_RGB16_PACK ( img->cs ) )
	{
		img->a[0] = malloc(img->s[0]*img->e[0]*sizeof(unsigned char) * 2 );
	}
	else if ( SCMN_CS_IS_RGB24_PACK ( img->cs ) )
	{
		img->a[0] = malloc(img->s[0]*img->e[0]*sizeof(unsigned char) * 3);
	}
	else if ( SCMN_CS_IS_RGB32_PACK ( img->cs ) )
	{
		img->a[0] = malloc(img->s[0]*img->e[0]*sizeof(unsigned char) * 4);
	}
	else
	{
		printf ( "Fail to allocate memory.\n" );
		ret = -1;
	}

	if ( !img->a[0]  )
		ret = -1;

	return ret;
}


static void write_decoded_image ( const char *filename,  SCMN_IMGB imgb )
{
	printf ( "Write decoded image to %s.\n",  filename );

	FILE *fp = fopen( filename, "wb");
	fwrite ( imgb.a[0], sizeof(unsigned char), imgb.s[0]*imgb.e[0], fp );
	fwrite ( imgb.a[1], sizeof(unsigned char), imgb.s[1]*imgb.e[1], fp );
	fwrite ( imgb.a[2], sizeof(unsigned char), imgb.s[2]*imgb.e[2], fp );

	fclose ( fp );

	return;
}


static int decode_bmp ( const char * input_file, int width, int height,  const char * output_file )
{
    	SCMN_IMGB imgb;
	SBMPD_INIT_DSC init_dsc;
    	SBMPD bmpd_hnd = NULL;
    	SBMPD_STAT bmpd_stat = {0,};
    	SBMPD_INFO bmpd_info = {0,};

	FILE *input_fp = NULL;
    	unsigned char *filebuffer = NULL;
    	unsigned char *bufp = NULL;
    	unsigned int nread = 0, toread = 0, totalread = 0,  decoded_size = 0;
 	struct stat statbuf;

   	int sret = SBMP_OK;
    	int ret = 0;
	int err = 0;


    	/* initialize struct variable */
     	memset(&imgb, 0x00, sizeof(SCMN_IMGB));
     	memset(&init_dsc, 0x00, sizeof(SBMPD_INIT_DSC));
  	memset(&bmpd_stat, 0x00, sizeof(SBMPD_STAT));
    	memset(&bmpd_info, 0x00, sizeof(SBMPD_INFO));

    	/* init bmp codec */
    	sret = sbmp_init();
    	if (SBMP_IS_ERR(sret))
    	{
        	printf ( "Fail to init bmp library\n");
        	ret = -1;
        	goto exit;
    	}

	printf ( "Initialize bmp library is success.\n" );

    	/* read data from gif file */
    	input_fp = fopen( input_file, "rb");
    	if (input_fp == NULL)
    	{
        	printf ( "Fail to read bmp file \n" );
        	goto exit;
    	}

	fstat(fileno(input_fp), &statbuf);

    	if (!(statbuf.st_mode & S_IFREG))
    	{
        	printf ( "Input file is not regular file\n" );
		ret = -1;
        	goto exit;
    	}

    	filebuffer = malloc(statbuf.st_size);
    	if (!filebuffer)
    	{
      	  	printf ( "fail to allocate file buffer\n" );
		ret = -1;
        	goto exit;
    	}

	toread = statbuf.st_size;
    	totalread = 0;
    	bufp = filebuffer;

    	while ((nread = fread(bufp+totalread, 1, toread, input_fp)) > 0)
    	{
        	totalread += nread;
        	toread -= nread;
    	}

    	if ( totalread != statbuf.st_size )
    	{
        	printf ( "BMP read error, file size=%ld, read size=%d\n", statbuf.st_size, totalread );
 		ret = -1;
       	goto exit;
    	}

	printf ( "Reading bmp image is success.\n" );


    	/* set init data */
    	init_dsc.bitb.addr = filebuffer;
    	init_dsc.bitb.size = totalread;
    	init_dsc.bitb.mt = SCMN_MT_IMG_BMP;
    	init_dsc.use_accel = 0;

	/* create BMP decoder's handle */
    	/* create BMP decoder's handle */
    	bmpd_hnd = sbmpd_create(&init_dsc, &bmpd_info, &err);
    	if ( bmpd_hnd == NULL || SBMP_IS_ERR(err))
    	{
        	printf ( "fail in sbmpd_create, erro=%d\n", err );
        	ret = -1;
        	goto exit;
    	}

	printf ( "Create bmp decoder is success.\n" );

	/* malloc for decoded picture */
	if ( ( width > 0 ) && ( height > 0 ) )
	{
		/* Resize the image */
		imgb.w[0] = width;
	      	imgb.h[0] = height;
	}
	else
	{
		/* original image size */
		imgb.w[0] = bmpd_info.w;
		imgb.h[0] = bmpd_info.h;
	}
	imgb.cs = bmpd_info.cs;

	printf ( "%d x %d --------> %d x %d \n" , bmpd_info.w, bmpd_info.h , imgb.w[0], imgb.h[0]  );


	ret = malloc_decoded_picture ( &imgb );
	if ( ret == -1 )
	{
		printf ( "Fail to malloc decoded data\n");
		goto exit;
	}

	printf ( "Success to malloc decoded data.\n" );

	/* decoding gif image */
    	sret = sbmpd_decode(bmpd_hnd, &imgb, &bmpd_stat);
    	if (SBMP_IS_ERR(sret))
    	{
        	printf("Fail to decode, erro=%d\n", sret);
        	ret = -1;
        	goto exit;
    	}

    	if (!bmpd_stat.pa)
    	{
        	printf("decoded buffer is not available\n");
        	ret = -1;
        	goto exit;
    	}


	/* Write decoded image if output_file name is set. */
	if ( output_file )
	{
		printf ( "Write decoded image. \n" );
		write_decoded_image ( output_file, imgb );
	}


exit:
	if ( bufp ) {
		free ( bufp );
		bufp = NULL;
	}

	if ( input_fp )
		fclose ( input_fp );

    	if ( bmpd_hnd )
        	sbmpd_delete ( bmpd_hnd );


    	sbmp_deinit();

    	printf("ret=%d\n", ret);

	return ret;
}

static int decode_png ( const char * input_file, int width, int height,  const char * output_file )
{
    	SCMN_IMGB imgb;
	SPNGD_INIT_DSC init_dsc;
    	SPNGD pngd_hnd = NULL;
    	SPNGD_STAT pngd_stat = {0,};
    	SPNGD_INFO pngd_info = {0,};

	FILE *input_fp = NULL;
    	unsigned char *filebuffer = NULL;
    	unsigned char *bufp = NULL;
    	unsigned int nread = 0, toread = 0, totalread = 0,  decoded_size = 0;
 	struct stat statbuf;

   	int sret = SPNG_OK;
    	int ret = 0;
	int err = 0;


    	/* initialize struct variable */
     	memset(&imgb, 0x00, sizeof(SCMN_IMGB));
     	memset(&init_dsc, 0x00, sizeof(SPNGD_INIT_DSC));
  	memset(&pngd_stat, 0x00, sizeof(SPNGD_STAT));
    	memset(&pngd_info, 0x00, sizeof(SPNGD_INFO));

    	/* init png codec */
    	sret = spng_init();
    	if (SPNG_IS_ERR(sret))
    	{
        	printf ( "Fail to init png library\n");
        	ret = -1;
        	goto exit;
    	}

	printf ( "Initialize png library is success.\n" );

    	/* read data from png file */
    	input_fp = fopen( input_file, "rb");
    	if (input_fp == NULL)
    	{
        	printf ( "Fail to read png file \n" );
        	goto exit;
    	}

	fstat(fileno(input_fp), &statbuf);

    	if (!(statbuf.st_mode & S_IFREG))
    	{
        	printf ( "Input file is not regular file\n" );
        	goto exit;
    	}

    	filebuffer = malloc(statbuf.st_size);
    	if (!filebuffer)
    	{
      	  	printf ( "fail to allocate file buffer\n" );
        	goto exit;
    	}

	toread = statbuf.st_size;
    	totalread = 0;
    	bufp = filebuffer;

    	while ((nread = fread(bufp+totalread, 1, toread, input_fp)) > 0)
    	{
        	totalread += nread;
        	toread -= nread;
    	}

    	if (totalread != statbuf.st_size)
    	{
        	printf ( "PNG read error, file size=%ld, read size=%d\n", statbuf.st_size, totalread );
        	goto exit;
    	}

	printf ( "Reading png image is success.\n" );


    	/* set init data */
    	init_dsc.bitb.addr = filebuffer;
    	init_dsc.bitb.size = totalread;
    	init_dsc.bitb.mt = SCMN_MT_IMG_PNG;
    	init_dsc.use_accel = 0;

    	/* create PNG decoder's handle */
    	pngd_hnd = spngd_create(&init_dsc, &pngd_info, &err);
    	if (pngd_hnd == NULL || SPNG_IS_ERR(err))
    	{
        	printf ( "fail in spngd_create, erro=%d\n", err);
        	ret = -1;
        	goto exit;
    	}

	printf ( "Create png decoder is success.\n" );

	/* malloc for decoded picture */
	if ( ( width > 0 ) && ( height > 0 ) )
	{
		/* Resize the image */
		imgb.w[0] = width;
	      	imgb.h[0] = height;
	}
	else
	{
		/* original image size */
		imgb.w[0] = pngd_info.img_dsc[0].w;
		imgb.h[0] = pngd_info.img_dsc[0].h;
	}
	imgb.cs = pngd_info.cs;

	printf ( "%d x %d --------> %d x %d \n" , pngd_info.img_dsc[0].w, pngd_info.img_dsc[0].h, imgb.w[0], imgb.h[0]  );


	ret = malloc_decoded_picture ( &imgb );
	if ( ret == -1 )
	{
		printf ( "Fail to malloc decoded data\n");
		goto exit;
	}


	printf ( "Success to malloc decoded data.\n" );

	/* decoding png image */
    	sret = spngd_decode(pngd_hnd, 0,&imgb, &pngd_stat);
    	if (SPNG_IS_ERR(sret))
    	{
        	printf("Fail to decode, erro=%d\n", sret);
        	ret = -1;
        	goto exit;
    	}

    	printf("Success to decode, erro=%d\n", sret);

    	if (!pngd_stat.pa)
    	{
        	printf("decoded buffer is not available\n");
        	ret = -1;
        	goto exit;
    	}

	printf ( "Decoding success.\n" );


	/* Write decoded image if output_file name is set. */
	if ( output_file )
	{
		printf ( "Write decoded image. \n" );
		write_decoded_image ( output_file, imgb );
	}


exit:

	if ( bufp ) {
		free ( bufp );
		bufp = NULL;
	}

	if ( input_fp )
		fclose ( input_fp );

    	if ( pngd_hnd )
        	spngd_delete ( pngd_hnd );


    	spng_deinit();

    	printf("ret=%d\n", ret);

	return ret;
}

static int decode_gif ( const char * input_file, int width, int height,  const char * output_file )
{
    	SCMN_IMGB imgb;
	SGIFD_INIT_DSC init_dsc;
    	SGIFD gifd_hnd = NULL;
    	SGIFD_STAT gifd_stat = {0,};
    	SGIFD_INFO gifd_info = {0,};

	FILE *input_fp = NULL;
    	unsigned char *filebuffer = NULL;
    	unsigned char *bufp = NULL;
    	unsigned int nread = 0, toread = 0, totalread = 0,  decoded_size = 0;
 	struct stat statbuf;

   	int sret = SGIF_OK;
    	int ret = 0;
	int err = 0;


    	/* initialize struct variable */
     	memset(&imgb, 0x00, sizeof(SCMN_IMGB));
     	memset(&init_dsc, 0x00, sizeof(SGIFD_INIT_DSC));
  	memset(&gifd_stat, 0x00, sizeof(SGIFD_STAT));
    	memset(&gifd_info, 0x00, sizeof(SGIFD_INFO));

    	/* init gif codec */
    	sret = sgif_init();
    	if (SGIF_IS_ERR(sret))
    	{
        	printf ( "Fail to init gif library\n");
        	ret = -1;
        	goto exit;
    	}

	printf ( "Initialize gif library is success.\n" );

    	/* read data from gif file */
    	input_fp = fopen( input_file, "rb");
    	if (input_fp == NULL)
    	{
        	printf ( "Fail to read gif file \n" );
        	goto exit;
    	}

	fstat(fileno(input_fp), &statbuf);

    	if (!(statbuf.st_mode & S_IFREG))
    	{
        	printf ( "Input file is not regular file\n" );
        	goto exit;
    	}

    	filebuffer = malloc(statbuf.st_size);
    	if (!filebuffer)
    	{
      	  	printf ( "fail to allocate file buffer\n" );
        	goto exit;
    	}

	toread = statbuf.st_size;
    	totalread = 0;
    	bufp = filebuffer;

    	while ((nread = fread(bufp+totalread, 1, toread, input_fp)) > 0)
    	{
        	totalread += nread;
        	toread -= nread;
    	}

    	if (totalread != statbuf.st_size)
    	{
        	printf ( "GIF read error, file size=%ld, read size=%d\n", statbuf.st_size, totalread );
        	goto exit;
    	}

	printf ( "Reading gif image is success.\n" );


    	/* set init data */
    	init_dsc.bitb.addr = filebuffer;
    	init_dsc.bitb.size = totalread;
    	init_dsc.bitb.mt = SCMN_MT_IMG_GIF;
    	init_dsc.use_accel = 0;

    	/* create GIF decoder's handle */
    	gifd_hnd = sgifd_create(&init_dsc, &gifd_info, &err);
    	if (gifd_hnd == NULL || SGIF_IS_ERR(err))
    	{
        	printf ( "fail in sgifd_create, erro=%d\n", err);
        	ret = -1;
        	goto exit;
    	}

	printf ( "Create gif decoder is success.\n" );

	/* malloc for decoded picture */
	if ( ( width > 0 ) && ( height > 0 ) )
	{
		/* Resize the image */
		imgb.w[0] = width;
	      	imgb.h[0] = height;
	}
	else
	{
		/* original image size */
		imgb.w[0] = gifd_info.w;
		imgb.h[0] = gifd_info.h;
	}
	imgb.cs = gifd_info.cs;

	printf ( "%d x %d --------> %d x %d \n" , gifd_info.w, gifd_info.h , imgb.w[0], imgb.h[0]  );


	ret = malloc_decoded_picture ( &imgb );
	if ( ret == -1 )
	{
		printf ( "Fail to malloc decoded data\n");
		goto exit;
	}

	printf ( "Success to malloc decoded data.\n" );

	/* decoding gif image */
    	sret = sgifd_decode(gifd_hnd, 0,&imgb, &gifd_stat);
    	if (SGIF_IS_ERR(sret))
    	{
        	printf("Fail to decode, erro=%d\n", sret);
        	ret = -1;
        	goto exit;
    	}

    	printf("Success to decode, erro=%d\n", sret);

    	if (!gifd_stat.pa)
    	{
        	printf("decoded buffer is not available\n");
        	ret = -1;
        	goto exit;
    	}


	/* Write decoded image if output_file name is set. */
	if ( output_file )
	{
		printf ( "Write decoded image. \n" );
		write_decoded_image ( output_file, imgb );
	}


exit:

	if ( bufp ) {
		free ( bufp );
		bufp = NULL;
	}

	if ( input_fp )
		fclose ( input_fp );

    	if ( gifd_hnd )
        	sgifd_delete ( gifd_hnd );


    	sgif_deinit();

    	printf("ret=%d\n", ret);

	return ret;

}
static int decode_jpg ( const char * input_file, int width, int height,  const char * output_file )
{
    	SCMN_IMGB imgb;
	SJPGD_INIT_DSC init_dsc;
    	SJPGD jpgd_hnd = NULL;
    	SJPGD_STAT jpgd_stat = {0,};
    	SJPGD_INFO jpgd_info = {0,};

	FILE *input_fp = NULL;
    	unsigned char *filebuffer = NULL;
    	unsigned char *bufp = NULL;
    	unsigned int nread = 0, toread = 0, totalread = 0,  decoded_size = 0;
 	struct stat statbuf;

   	int sret = SJPG_OK;
    	int ret = 0;
	int err = 0;


    	/* initialize struct variable */
     	memset(&imgb, 0x00, sizeof(SCMN_IMGB));
     	memset(&init_dsc, 0x00, sizeof(SJPGD_INIT_DSC));
  	memset(&jpgd_stat, 0x00, sizeof(SJPGD_STAT));
    	memset(&jpgd_info, 0x00, sizeof(SJPGD_INFO));

    	/* init jpg codec */
    	sret = sjpg_init();
    	if (SJPG_IS_ERR(sret))
    	{
        	printf ( "Fail to init jpg library\n");
        	ret = -1;
        	goto exit;
    	}

	printf ( "Initialize jpg library is success.\n" );

    	/* read data from jpg file */
    	input_fp = fopen( input_file, "rb");
    	if (input_fp == NULL)
    	{
        	printf ( "Fail to read jpg file \n" );
        	goto exit;
    	}

	fstat(fileno(input_fp), &statbuf);

    	if (!(statbuf.st_mode & S_IFREG))
    	{
        	printf ( "Input file is not regular file\n" );
        	goto exit;
    	}

    	filebuffer = malloc(statbuf.st_size);
    	if (!filebuffer)
    	{
      	  	printf ( "fail to allocate file buffer\n" );
        	goto exit;
    	}

	toread = statbuf.st_size;
    	totalread = 0;
    	bufp = filebuffer;

    	while ((nread = fread(bufp+totalread, 1, toread, input_fp)) > 0)
    	{
        	totalread += nread;
        	toread -= nread;
    	}

    	if (totalread != statbuf.st_size)
    	{
        	printf ( "JPG read error, file size=%ld, read size=%d\n", statbuf.st_size, totalread );
        	goto exit;
    	}

	printf ( "Reading jpg image is success.\n" );


    	/* set init data */
    	init_dsc.bitb.addr = filebuffer;
    	init_dsc.bitb.size = totalread;
    	init_dsc.bitb.mt = SCMN_MT_IMG_JPG;
    	init_dsc.use_accel = 0;

    	/* create JPG decoder's handle */
    	jpgd_hnd = sjpgd_create(&init_dsc, &jpgd_info, &err);
    	if (jpgd_hnd == NULL || SJPG_IS_ERR(err))
    	{
        	printf ( "fail in sjpgd_create, erro=%d\n", err);
        	ret = -1;
        	goto exit;
    	}

	printf ( "Create jpg decoder is success.\n" );

	/* malloc for decoded picture */
	if ( ( width > 0 ) && ( height > 0 ) )
	{
		/* Resize the image */
		imgb.w[0] = width;
	      	imgb.h[0] = height;
	}
	else
	{
		/* original image size */
		imgb.w[0] = jpgd_info.w;
		imgb.h[0] = jpgd_info.h;
	}
	imgb.cs = jpgd_info.cs;

	printf ( "%d x %d --------> %d x %d \n" , jpgd_info.w, jpgd_info.h , imgb.w[0], imgb.h[0]  );


	ret = malloc_decoded_picture ( &imgb );
	if ( ret == -1 )
	{
		printf ( "Fail to malloc decoded data\n");
		goto exit;
	}

	printf ( "Success to malloc decoded data.\n" );


	/* decoding jpg image */
    	sret = sjpgd_decode(jpgd_hnd, &imgb, &jpgd_stat);
    	if (SJPG_IS_ERR(sret))
    	{
        	printf("Fail to decode, erro=%d\n", sret);
        	ret = -1;
        	goto exit;
    	}

    	if (!jpgd_stat.pa)
    	{
        	printf("decoded buffer is not available\n");
        	ret = -1;
        	goto exit;
    	}

	printf ( "jpgd_stat.w=%d, jpgd_stat.h=%d, jpgd_stat.read=%d \n", jpgd_stat.w, jpgd_stat.h, jpgd_stat.read );

	/* Write decoded image if output_file name is set. */
	if ( output_file )
	{
		printf ( "Write decoded image. \n" );
		write_decoded_image ( output_file, imgb );
	}


exit:
	if ( bufp ) {
		free ( bufp );
		bufp = NULL;
	}

	if ( input_fp )
		fclose ( input_fp );

    	if ( jpgd_hnd )
        	sjpgd_delete ( jpgd_hnd );


    	sjpg_deinit();

    	printf("ret=%d\n", ret);

	return ret;
}

int main(int argc, char *argv[])
{

	mm_utility_image_file_format file_format;
	char *input_file, *output_file;
	int output_width, output_height;
    	int ret = 0;

	file_format = MM_UTILITY_IMAGE_FILE_FMT_NONE;
	input_file = output_file = NULL;
	output_width = output_height = 0;

	/* Check image file path */
	if ( !argv[1] )
	{
		printf ( "Please input image file path. \n" );
		goto exit;
	}

	input_file = g_strdup ( (const char *) argv[1] );

	printf ( "Input file name is %s\n", input_file );

	/* Check outpue decoded image size. if not set, use original image size. */
	if ( argv[2] && argv[3] )
	{
		output_width = atoi(argv[2]);
		output_height = atoi(argv[3]);
		
		printf ( "Resize decoded image : %dX%d\n", output_width, output_height );
	}

	/* Check output decoded image file path */
	if ( argc>3 && argv[4] )
	{
		output_file = g_strdup ( (const char *)argv[4] );
		printf ( "Output decoded image file path [%s]. \n",  output_file );
	}

	/* Get image file format : bmp, gif, png, jpg */
	file_format = get_file_format ( input_file );


	/* Decode image file */
	switch ( file_format )
	{
		case MM_UTILITY_IMAGE_FILE_FMT_BMP:
			ret = decode_bmp ( input_file, output_width, output_height, output_file );
			break;
		case MM_UTILITY_IMAGE_FILE_FMT_GIF:
			ret = decode_gif ( input_file, output_width, output_height, output_file );
			break;
		case MM_UTILITY_IMAGE_FILE_FMT_PNG:
			ret = decode_png ( input_file, output_width, output_height, output_file);
			break;
		case MM_UTILITY_IMAGE_FILE_FMT_JPG:
			ret = decode_jpg ( input_file, output_width, output_height, output_file );
			break;
		default:
			printf ( " This image file is not supported. \n" );
			ret = -1;
			break;
			
	}

	if ( ret == -1 )
		printf ( "Fail to decode [%s]. \n", input_file );

exit:
	if ( input_file )
	{
		g_free ( input_file );
		input_file = NULL;
	}

	if ( output_file )
	{
		g_free ( output_file );
		output_file = NULL;
	}
	
	return ret;

}


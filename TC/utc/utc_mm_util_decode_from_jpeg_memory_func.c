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

/**
* @ingroup	MMF_UTIL_API
* @addtogroup	UTIL
*/

/**
* @ingroup	UTIL
* @addtogroup	UTS_MMF_UTIL Unit
*/

/**
* @ingroup	UTS_MMF_UTIL Unit
* @addtogroup	UTS_MMF_UTIL_DECODE_FROM_JPEG_MEMORY Uts_Mmf_Util_Decode_From_Jpeg_Memory
* @{
*/

/**
* @file uts_mm_util_decode_from_jpeg_memory.c
* @brief This is a suit of unit test cases to test mm_util_decode_from_jpeg_memory API
* @version Initial Creation Version 0.1
* @date 2010.07.21
*/


#include "utc_mm_util_common.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////
// Declare the global variables and registers and Internal Funntions
//-------------------------------------------------------------------------------------------------
void *data;

struct tet_testlist tet_testlist[] = {
	{utc_mm_util_decode_from_jpeg_memory_func_01 , 1},
	{utc_mm_util_decode_from_jpeg_memory_func_02 , 2},
	{utc_mm_util_decode_from_jpeg_memory_func_03 , 3},
	{utc_mm_util_decode_from_jpeg_memory_func_04 , 4},
	{utc_mm_util_decode_from_jpeg_memory_func_05 , 5},
	{utc_mm_util_decode_from_jpeg_memory_func_06 , 6},
	{NULL, 0}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/* Initialize TCM data structures */

/* Start up function for each test purpose */
void
startup ()
{
	tet_infoline("[[ COMMON ]]::Inside startup \n");

	tet_infoline("[[ COMMON ]]::Completing startup \n");
}

/* Clean up function for each test purpose */
void
cleanup ()
{
}

void utc_mm_util_decode_from_jpeg_memory_func_01()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 50;
	unsigned int src_height = 50;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	mm_util_jpeg_yuv_data decoded;

	FILE *fp = fopen(JPG_IMAGE_FILE, "r");

	tet_infoline( "[[ TET_MSG ]]:: Extract YUV data from JPEG buffer" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_decode_from_jpeg_memory(&decoded, src, src_size, src_format);

	dts_check_eq(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_decode_from_jpeg_memory_func_02()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 50;
	unsigned int src_height = 50;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	//mm_util_jpeg_yuv_data decoded;

	FILE *fp = fopen(JPG_IMAGE_FILE, "r");

	tet_infoline( "[[ TET_MSG ]]:: Extract YUV data from JPEG buffer" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_decode_from_jpeg_memory(NULL, src, src_size, src_format);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_decode_from_jpeg_memory_func_03()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 50;
	unsigned int src_height = 50;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	mm_util_jpeg_yuv_data decoded;

	FILE *fp = fopen(JPG_IMAGE_FILE, "r");

	tet_infoline( "[[ TET_MSG ]]:: Extract YUV data from JPEG buffer" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_decode_from_jpeg_memory(&decoded, NULL, src_size, src_format);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_decode_from_jpeg_memory_func_04()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 50;
	unsigned int src_height = 50;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	mm_util_jpeg_yuv_data decoded;

	FILE *fp = fopen(JPG_IMAGE_FILE, "r");

	tet_infoline( "[[ TET_MSG ]]:: Extract YUV data from JPEG buffer" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_decode_from_jpeg_memory(&decoded, src, -1, src_format);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_decode_from_jpeg_memory_func_05()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 50;
	unsigned int src_height = 50;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	mm_util_jpeg_yuv_data decoded;

	FILE *fp = fopen(JPG_IMAGE_FILE, "r");

	tet_infoline( "[[ TET_MSG ]]:: Extract YUV data from JPEG buffer" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_decode_from_jpeg_memory(&decoded, src, src_size, -1);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_decode_from_jpeg_memory_func_06()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 50;
	unsigned int src_height = 50;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	mm_util_jpeg_yuv_data decoded;

	FILE *fp = fopen(JPG_IMAGE_FILE, "r");

	tet_infoline( "[[ TET_MSG ]]:: Extract YUV data from JPEG buffer" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_decode_from_jpeg_memory(&decoded, src, src_size, 100);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


/** @} */





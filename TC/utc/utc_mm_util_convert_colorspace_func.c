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
* @addtogroup	UTS_MMF_UTIL_CONVERT_COLORSPACE Uts_Mmf_Util_Convert_Colorspace
* @{
*/

/**
* @file uts_mm_util_convert_colorspace.c
* @brief This is a suit of unit test cases to test mm_util_convert_colorspace API
* @version Initial Creation Version 0.1
* @date 2010.06.08
*/


#include "utc_mm_util_common.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////
// Declare the global variables and registers and Internal Funntions
//-------------------------------------------------------------------------------------------------
void *data;

struct tet_testlist tet_testlist[] = {
	{utc_mm_util_convert_colorspace_func_01 , 1},
	{utc_mm_util_convert_colorspace_func_02 , 2},
	{utc_mm_util_convert_colorspace_func_03 , 3},
	{utc_mm_util_convert_colorspace_func_04 , 4},
	{utc_mm_util_convert_colorspace_func_05 , 5},
	{utc_mm_util_convert_colorspace_func_06 , 6},
	{utc_mm_util_convert_colorspace_func_07 , 7},
	{utc_mm_util_convert_colorspace_func_08 , 8},
	{utc_mm_util_convert_colorspace_func_09 , 9},
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

void utc_mm_util_convert_colorspace_func_01()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst;
	unsigned int dst_width = 100;
	unsigned int dst_height = 100;
	unsigned int dst_size;
	mm_util_img_format dst_format = MM_UTIL_IMG_FMT_RGB888;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Convert colorspace of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_get_image_size(dst_format, dst_width, dst_height, &dst_size);
	dst = malloc(dst_size);

	ret = mm_util_convert_colorspace(src, src_width, src_height, src_format,dst, dst_format);

	dts_check_eq(__func__, ret, MM_ERROR_NONE, "err=%x", ret );


	free(src);
	free(dst);
	fclose(fp);
	return;
}


void utc_mm_util_convert_colorspace_func_02()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst;
	unsigned int dst_width = 100;
	unsigned int dst_height = 100;
	unsigned int dst_size;
	mm_util_img_format dst_format = MM_UTIL_IMG_FMT_RGB888;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Convert colorspace of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_get_image_size(dst_format, dst_width, dst_height, &dst_size);
	dst = malloc(dst_size);

	ret = mm_util_convert_colorspace(NULL, src_width, src_height, src_format,dst, dst_format);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	free(dst);
	fclose(fp);
	return;
}


void utc_mm_util_convert_colorspace_func_03()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst;
	unsigned int dst_width = 100;
	unsigned int dst_height = 100;
	unsigned int dst_size;
	mm_util_img_format dst_format = MM_UTIL_IMG_FMT_RGB888;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Convert colorspace of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_get_image_size(dst_format, dst_width, dst_height, &dst_size);
	dst = malloc(dst_size);

	ret = mm_util_convert_colorspace(src, -1, src_height, src_format,dst, dst_format);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	free(dst);
	fclose(fp);
	return;
}


void utc_mm_util_convert_colorspace_func_04()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst;
	unsigned int dst_width = 100;
	unsigned int dst_height = 100;
	unsigned int dst_size;
	mm_util_img_format dst_format = MM_UTIL_IMG_FMT_RGB888;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Convert colorspace of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_get_image_size(dst_format, dst_width, dst_height, &dst_size);
	dst = malloc(dst_size);

	ret = mm_util_convert_colorspace(src, src_width, -1, src_format,dst, dst_format);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	free(dst);
	fclose(fp);
	return;
}


void utc_mm_util_convert_colorspace_func_05()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst;
	unsigned int dst_width = 100;
	unsigned int dst_height = 100;
	unsigned int dst_size;
	mm_util_img_format dst_format = MM_UTIL_IMG_FMT_RGB888;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Convert colorspace of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_get_image_size(dst_format, dst_width, dst_height, &dst_size);
	dst = malloc(dst_size);

	ret = mm_util_convert_colorspace(src, src_width, src_height, -1,dst, dst_format);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	free(dst);
	fclose(fp);
	return;
}


void utc_mm_util_convert_colorspace_func_06()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst;
	unsigned int dst_width = 100;
	unsigned int dst_height = 100;
	unsigned int dst_size;
	mm_util_img_format dst_format = MM_UTIL_IMG_FMT_RGB888;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Convert colorspace of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_get_image_size(dst_format, dst_width, dst_height, &dst_size);
	dst = malloc(dst_size);

	ret = mm_util_convert_colorspace(src, src_width, src_height, 20,dst, dst_format);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	free(dst);
	fclose(fp);
	return;
}


void utc_mm_util_convert_colorspace_func_07()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst;
	unsigned int dst_width = 100;
	unsigned int dst_height = 100;
	unsigned int dst_size;
	mm_util_img_format dst_format = MM_UTIL_IMG_FMT_RGB888;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Convert colorspace of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_get_image_size(dst_format, dst_width, dst_height, &dst_size);
	dst = malloc(dst_size);

	ret = mm_util_convert_colorspace(src, src_width, src_height, src_format,NULL, dst_format);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	free(dst);
	fclose(fp);
	return;
}


void utc_mm_util_convert_colorspace_func_08()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst;
	unsigned int dst_width = 100;
	unsigned int dst_height = 100;
	unsigned int dst_size;
	mm_util_img_format dst_format = MM_UTIL_IMG_FMT_RGB888;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Convert colorspace of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_get_image_size(dst_format, dst_width, dst_height, &dst_size);
	dst = malloc(dst_size);

	ret = mm_util_convert_colorspace(src, src_width, src_height, src_format,dst, -1);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	free(dst);
	fclose(fp);
	return;
}


void utc_mm_util_convert_colorspace_func_09()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst;
	unsigned int dst_width = 100;
	unsigned int dst_height = 100;
	unsigned int dst_size;
	mm_util_img_format dst_format = MM_UTIL_IMG_FMT_RGB888;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Convert colorspace of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	ret = mm_util_get_image_size(dst_format, dst_width, dst_height, &dst_size);
	dst = malloc(dst_size);

	ret = mm_util_convert_colorspace(src, src_width, src_height, src_format,dst, 20);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	free(dst);
	fclose(fp);
	return;
}



/** @} */





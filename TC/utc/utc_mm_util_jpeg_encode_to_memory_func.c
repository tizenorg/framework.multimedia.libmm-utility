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
* @addtogroup	UTS_MMF_UTIL_JPEG_ENCODE_TO_MEMORY Uts_Mmf_Util_Jpeg_Encode_To_Memory
* @{
*/

/**
* @file uts_mm_util_jpeg_encode_to_memory.c
* @brief This is a suit of unit test cases to test mm_util_jpeg_encode_to_memory API
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
	{utc_mm_util_jpeg_encode_to_memory_func_01 , 1},
	{utc_mm_util_jpeg_encode_to_memory_func_02 , 2},
	{utc_mm_util_jpeg_encode_to_memory_func_03 , 3},
	{utc_mm_util_jpeg_encode_to_memory_func_04 , 4},
	{utc_mm_util_jpeg_encode_to_memory_func_05 , 5},
	{utc_mm_util_jpeg_encode_to_memory_func_06 , 6},
	{utc_mm_util_jpeg_encode_to_memory_func_07 , 7},
	{utc_mm_util_jpeg_encode_to_memory_func_08 , 8},
	{utc_mm_util_jpeg_encode_to_memory_func_09 , 9},
	{utc_mm_util_jpeg_encode_to_memory_func_10 , 10},
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

void utc_mm_util_jpeg_encode_to_memory_func_01()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	int size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	mem = malloc(src_size);
	size = (int)src_size;
	ret = mm_util_jpeg_encode_to_memory(&mem, &size, src, src_width, src_height, src_format, quality);

	dts_check_eq(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_jpeg_encode_to_memory_func_02()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	int size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	//void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);
	size = (int)src_size;
	//mem = malloc(src_size);
	ret = mm_util_jpeg_encode_to_memory(NULL, &size, src, src_width, src_height, src_format, quality);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_jpeg_encode_to_memory_func_03()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	mem = malloc(src_size);
	ret = mm_util_jpeg_encode_to_memory(&mem, NULL, src, src_width, src_height, src_format, quality);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_jpeg_encode_to_memory_func_04()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	int size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	mem = malloc(src_size);
	size = (int)src_size;
	ret = mm_util_jpeg_encode_to_memory(&mem, &size, NULL, src_width, src_height, src_format, quality);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_jpeg_encode_to_memory_func_05()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	int size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	mem = malloc(src_size);
	size = (int)src_size;
	ret = mm_util_jpeg_encode_to_memory(&mem, &size, src, -1, src_height, src_format, quality);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_jpeg_encode_to_memory_func_06()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	int size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	mem = malloc(src_size);
	size = (int)src_size;
	ret = mm_util_jpeg_encode_to_memory(&mem, &size, src, src_width, -1, src_format, quality);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_jpeg_encode_to_memory_func_07()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	int size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	mem = malloc(src_size);
	size = (int)src_size;
	ret = mm_util_jpeg_encode_to_memory(&mem, &size, src, src_width, src_height, -1, quality);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_jpeg_encode_to_memory_func_08()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	int size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	mem = malloc(src_size);
	size = (int)src_size;
	ret = mm_util_jpeg_encode_to_memory(&mem, &size, src, src_width, src_height, 100, quality);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_jpeg_encode_to_memory_func_09()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	int size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	mem = malloc(src_size);
	size = (int)src_size;
	ret = mm_util_jpeg_encode_to_memory(&mem, &size, src, src_width, src_height, src_format, -1);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	free(src);
	fclose(fp);
	return;
}


void utc_mm_util_jpeg_encode_to_memory_func_10()
{
	int ret = 0;
	unsigned char *src;
	unsigned int src_width = 100;
	unsigned int src_height = 100;
	unsigned int src_size;
	int size;
	mm_util_img_format src_format = MM_UTIL_IMG_FMT_YUV420;
	int quality = 50;
	void* mem;

	FILE *fp = fopen(YUV_IMAGE_FILE, "rb");

	tet_infoline( "[[ TET_MSG ]]:: Encode raw file into memory" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(src_format, src_width, src_height, &src_size);
	src = malloc(src_size);
	fread(src, 1, src_size, fp);

	mem = malloc(src_size);
	size = (int)src_size;
	ret = mm_util_jpeg_encode_to_memory(&mem, &size, src, src_width, src_height, src_format, 200);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );


	free(src);
	fclose(fp);
	return;
}



/** @} */





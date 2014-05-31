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
* @addtogroup	UTS_MMF_UTIL_GET_IMAGE_SIZE Uts_Mmf_Util_Get_Image_Size
* @{
*/

/**
* @file uts_mm_util_get_image_size.c
* @brief This is a suit of unit test cases to test mm_util_get_image_size API
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
	{utc_mm_util_get_image_size_func_01 , 1},
	{utc_mm_util_get_image_size_func_02 , 2},
	{utc_mm_util_get_image_size_func_03 , 3},
	{utc_mm_util_get_image_size_func_04 , 4},
	{utc_mm_util_get_image_size_func_05 , 5},
	{utc_mm_util_get_image_size_func_06 , 6},
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

void utc_mm_util_get_image_size_func_01()
{
	int ret = 0;
	mm_util_img_format format = MM_UTIL_IMG_FMT_YUV420;
	unsigned int width = 50;
	unsigned int height = 50;
	unsigned int size;

	tet_infoline( "[[ TET_MSG ]]:: Get size of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(format, width, height, &size);

	dts_check_eq(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	return;
}


void utc_mm_util_get_image_size_func_02()
{
	int ret = 0;
	mm_util_img_format format = -1;
	unsigned int width = 50;
	unsigned int height = 50;
	unsigned int size;

	tet_infoline( "[[ TET_MSG ]]:: Get size of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(format, width, height, &size);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	return;
}


void utc_mm_util_get_image_size_func_03()
{
	int ret = 0;
	mm_util_img_format format = 20;
	unsigned int width = 50;
	unsigned int height = 50;
	unsigned int size;

	tet_infoline( "[[ TET_MSG ]]:: Get size of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(format, width, height, &size);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	return;
}


void utc_mm_util_get_image_size_func_04()
{
	int ret = 0;
	mm_util_img_format format = MM_UTIL_IMG_FMT_YUV420;
	unsigned int width = -50;
	unsigned int height = 50;
	unsigned int size;

	tet_infoline( "[[ TET_MSG ]]:: Get size of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(format, width, height, &size);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	return;
}


void utc_mm_util_get_image_size_func_05()
{
	int ret = 0;
	mm_util_img_format format = MM_UTIL_IMG_FMT_YUV420;
	unsigned int width = 50;
	unsigned int height = -50;
	unsigned int size;

	tet_infoline( "[[ TET_MSG ]]:: Get size of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(format, width, height, &size);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	return;
}


void utc_mm_util_get_image_size_func_06()
{
	int ret = 0;
	mm_util_img_format format = MM_UTIL_IMG_FMT_YUV420;
	unsigned int width = 50;
	unsigned int height = 50;
	unsigned int size;

	tet_infoline( "[[ TET_MSG ]]:: Get size of the Image" );

	/* Get the data of the Image */
	ret = mm_util_get_image_size(format, width, height, NULL);

	dts_check_ne(__func__, ret, MM_ERROR_NONE, "err=%x", ret );

	return;
}



/** @} */





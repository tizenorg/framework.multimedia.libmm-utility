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


#ifndef UTS_MM_FRAMEWORK_UTIL_COMMON_H
#define UTS_MM_FRAMEWORK_UTIL_COMMON_H

#include <mm_util_imgp.h>
#include <mm_util_jpeg.h>
#include <mm_message.h>
#include <mm_error.h>
#include <mm_types.h>
#include <stdio.h>
#include <string.h>
#include <tet_api.h>
#include <unistd.h>
#include <glib-2.0/glib.h>

void startup();
void cleanup();

void (*tet_startup)() = startup;
void (*tet_cleanup)() = cleanup;



/* Audio File Define */
#define JPG_IMAGE_FILE "test_data/test.jpg"
#define YUV_IMAGE_FILE "test_data/test.yuv"


void utc_mm_util_get_image_size_func_01 ();
void utc_mm_util_get_image_size_func_02 ();
void utc_mm_util_get_image_size_func_03 ();
void utc_mm_util_get_image_size_func_04 ();
void utc_mm_util_get_image_size_func_05 ();
void utc_mm_util_get_image_size_func_06 ();

void utc_mm_util_convert_colorspace_func_01 ();
void utc_mm_util_convert_colorspace_func_02 ();
void utc_mm_util_convert_colorspace_func_03 ();
void utc_mm_util_convert_colorspace_func_04 ();
void utc_mm_util_convert_colorspace_func_05 ();
void utc_mm_util_convert_colorspace_func_06 ();
void utc_mm_util_convert_colorspace_func_07 ();
void utc_mm_util_convert_colorspace_func_08 ();
void utc_mm_util_convert_colorspace_func_09 ();

void utc_mm_util_resize_image_func_01 ();
void utc_mm_util_resize_image_func_02 ();
void utc_mm_util_resize_image_func_03 ();
void utc_mm_util_resize_image_func_04 ();
void utc_mm_util_resize_image_func_05 ();
void utc_mm_util_resize_image_func_06 ();
void utc_mm_util_resize_image_func_07 ();
void utc_mm_util_resize_image_func_08 ();
void utc_mm_util_resize_image_func_09 ();

void utc_mm_util_rotate_image_func_01 ();
void utc_mm_util_rotate_image_func_02 ();
void utc_mm_util_rotate_image_func_03 ();
void utc_mm_util_rotate_image_func_04 ();
void utc_mm_util_rotate_image_func_05 ();
void utc_mm_util_rotate_image_func_06 ();
void utc_mm_util_rotate_image_func_07 ();
void utc_mm_util_rotate_image_func_08 ();
void utc_mm_util_rotate_image_func_09 ();
void utc_mm_util_rotate_image_func_10 ();
void utc_mm_util_rotate_image_func_11 ();

void utc_mm_util_jpeg_encode_to_file_func_01 ();
void utc_mm_util_jpeg_encode_to_file_func_02 ();
void utc_mm_util_jpeg_encode_to_file_func_03 ();
void utc_mm_util_jpeg_encode_to_file_func_04 ();
void utc_mm_util_jpeg_encode_to_file_func_05 ();
void utc_mm_util_jpeg_encode_to_file_func_06 ();
void utc_mm_util_jpeg_encode_to_file_func_07 ();
void utc_mm_util_jpeg_encode_to_file_func_08 ();
void utc_mm_util_jpeg_encode_to_file_func_09 ();

void utc_mm_util_jpeg_encode_to_memory_func_01 ();
void utc_mm_util_jpeg_encode_to_memory_func_02 ();
void utc_mm_util_jpeg_encode_to_memory_func_03 ();
void utc_mm_util_jpeg_encode_to_memory_func_04 ();
void utc_mm_util_jpeg_encode_to_memory_func_05 ();
void utc_mm_util_jpeg_encode_to_memory_func_06 ();
void utc_mm_util_jpeg_encode_to_memory_func_07 ();
void utc_mm_util_jpeg_encode_to_memory_func_08 ();
void utc_mm_util_jpeg_encode_to_memory_func_09 ();
void utc_mm_util_jpeg_encode_to_memory_func_10 ();

void utc_mm_util_decode_from_jpeg_file_func_01 ();
void utc_mm_util_decode_from_jpeg_file_func_02 ();
void utc_mm_util_decode_from_jpeg_file_func_03 ();
void utc_mm_util_decode_from_jpeg_file_func_04 ();
void utc_mm_util_decode_from_jpeg_file_func_05 ();

void utc_mm_util_decode_from_jpeg_memory_func_01 ();
void utc_mm_util_decode_from_jpeg_memory_func_02 ();
void utc_mm_util_decode_from_jpeg_memory_func_03 ();
void utc_mm_util_decode_from_jpeg_memory_func_04 ();
void utc_mm_util_decode_from_jpeg_memory_func_05 ();
void utc_mm_util_decode_from_jpeg_memory_func_06 ();


#endif /* UTS_MM_FRAMEWORK_UTIL_COMMON_H */

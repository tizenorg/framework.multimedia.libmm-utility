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
#include <limits.h>
#include <mm_debug.h>
#include "mm_util_imgp.h"
#include "mm_util_imgp_internal.h"
#include <gmodule.h>
#include <mm_error.h>
#ifdef ENABLE_TTRACE
#include <ttrace.h>
#define TTRACE_BEGIN(NAME) traceBegin(TTRACE_TAG_IMAGE, NAME)
#define TTRACE_END() traceEnd(TTRACE_TAG_IMAGE)
#else //ENABLE_TTRACE
#define TTRACE_BEGIN(NAME)
#define TTRACE_END()
#endif //ENABLE_TTRACE

#define MM_UTIL_ROUND_UP_2(num) (((num)+1)&~1)
#define MM_UTIL_ROUND_UP_4(num) (((num)+3)&~3)
#define MM_UTIL_ROUND_UP_8(num) (((num)+7)&~7)
#define MM_UTIL_ROUND_UP_16(num) (((num)+15)&~15)
#define GEN_MASK(x) ((1<<(x))-1)
#define ROUND_UP_X(v,x) (((v) + GEN_MASK(x)) & ~GEN_MASK(x))
#define DIV_ROUND_UP_X(v,x) (((v) + GEN_MASK(x)) >> (x))
#define GST "gstcs"

typedef gboolean(*IMGPInfoFunc) (imgp_info_s*, const unsigned char*, unsigned char*, imgp_plugin_type_e);
static int __mm_util_transform_exec(mm_util_s * handle, media_packet_h src_packet);

static int check_valid_picture_size(int width, int height)
{
	if((int)width>0 && (int)height>0 && (width+128)*(unsigned long long)(height+128) < INT_MAX/4) {
		return MM_ERROR_NONE;
	}

	return MM_ERROR_IMAGE_INVALID_VALUE;
}

static void __mm_destroy_temp_buffer(unsigned char *buffer[])
{
	int i = 0;

	for(i = 0; i < 4; i++) {
		IMGP_FREE(buffer[i]);
	}
}

static gboolean __mm_cannot_convert_format(mm_util_img_format src_format, mm_util_img_format dst_format )
{
	gboolean _bool=FALSE;

	debug_log("src_format: %d, dst_format:%d", src_format, dst_format);

	if((dst_format == MM_UTIL_IMG_FMT_NV16) || (dst_format == MM_UTIL_IMG_FMT_NV61) ||
		((src_format == MM_UTIL_IMG_FMT_YUV422) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||
		((src_format == MM_UTIL_IMG_FMT_UYVY) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||
		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||
		((src_format == MM_UTIL_IMG_FMT_RGB565) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||
		((src_format == MM_UTIL_IMG_FMT_RGB888) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||
		((src_format == MM_UTIL_IMG_FMT_BGRX8888) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_YUV422)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_UYVY)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_YUYV)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_BGRX8888)) ) {

		_bool = TRUE;
	}

	return _bool;
}

static gboolean __mm_gst_can_resize_format(char* __format_label)
{
	gboolean _bool = FALSE;

	debug_log("Format label: %s",__format_label);

	if(strcmp(__format_label, "AYUV") == 0
		|| strcmp(__format_label, "UYVY") == 0 ||strcmp(__format_label, "Y800") == 0 || strcmp(__format_label, "I420") == 0 || strcmp(__format_label, "YV12") == 0
		|| strcmp(__format_label, "RGB888") == 0 || strcmp(__format_label, "RGB565") == 0 || strcmp(__format_label, "BGR888") == 0 || strcmp(__format_label, "RGBA8888") == 0
		|| strcmp(__format_label, "ARGB8888") == 0 ||strcmp(__format_label, "BGRA8888") == 0 || strcmp(__format_label, "ABGR8888") == 0 || strcmp(__format_label, "RGBX") == 0
		|| strcmp(__format_label, "XRGB") == 0 || strcmp(__format_label, "BGRX") == 0 || strcmp(__format_label, "XBGR") == 0 || strcmp(__format_label, "Y444") == 0
		|| strcmp(__format_label, "Y42B") == 0 || strcmp(__format_label, "YUY2") == 0 || strcmp(__format_label, "YUYV") == 0 || strcmp(__format_label, "UYVY") == 0
		|| strcmp(__format_label, "Y41B") == 0 || strcmp(__format_label, "Y16") == 0 || strcmp(__format_label, "Y800") == 0 || strcmp(__format_label, "Y8") == 0
		|| strcmp(__format_label, "GREY") == 0 || strcmp(__format_label, "AY64") == 0 || strcmp(__format_label, "YUV422") == 0) {

		_bool=TRUE;
	}

	return _bool;
}

static gboolean __mm_gst_can_rotate_format(const char* __format_label)
{
	gboolean _bool = FALSE;

	debug_log("Format label: %s boolean: %d", __format_label, _bool);

	if(strcmp(__format_label, "I420") == 0 ||strcmp(__format_label, "YV12") == 0 || strcmp(__format_label, "IYUV") == 0
		|| strcmp(__format_label, "RGB888") == 0||strcmp(__format_label, "BGR888") == 0 ||strcmp(__format_label, "RGBA8888") == 0
		|| strcmp(__format_label, "ARGB8888") == 0 ||strcmp(__format_label, "BGRA8888") == 0 ||strcmp(__format_label, "ABGR8888") == 0 ) {

		_bool=TRUE;
	}

	debug_log("boolean: %d",_bool);

	return _bool;
}

static gboolean __mm_select_convert_plugin(mm_util_img_format src_format, mm_util_img_format dst_format )
{
	gboolean _bool=FALSE;

	debug_log("src_format: %d, dst_format:%d", src_format, dst_format);

	if(((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||
		((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||
		((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||
		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||
		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||
		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) ||
		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||

		((src_format == MM_UTIL_IMG_FMT_UYVY) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||
		((src_format == MM_UTIL_IMG_FMT_UYVY) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||

		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||
		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||

		((src_format == MM_UTIL_IMG_FMT_RGB565) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) ||
		((src_format == MM_UTIL_IMG_FMT_RGB565) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_RGB565) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||

		((src_format == MM_UTIL_IMG_FMT_RGB888) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) ||
		((src_format == MM_UTIL_IMG_FMT_RGB888) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_RGB888) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||

		((src_format == MM_UTIL_IMG_FMT_ARGB8888) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) ||
		((src_format == MM_UTIL_IMG_FMT_ARGB8888) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_ARGB8888) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||

		((src_format == MM_UTIL_IMG_FMT_BGRA8888) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) ||
		((src_format == MM_UTIL_IMG_FMT_BGRA8888) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_BGRA8888) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||
		((src_format == MM_UTIL_IMG_FMT_RGBA8888) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) ||
		((src_format == MM_UTIL_IMG_FMT_RGBA8888) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_RGBA8888) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||
		((src_format == MM_UTIL_IMG_FMT_RGBA8888) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||

		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888))) {

		_bool = TRUE;
	}

	return _bool;
}

static gboolean __mm_select_resize_plugin(mm_util_img_format _format)
{
	gboolean _bool = FALSE;

	debug_log("_format: %d", _format);

	if ((_format == MM_UTIL_IMG_FMT_UYVY) || (_format == MM_UTIL_IMG_FMT_YUYV) || (_format == MM_UTIL_IMG_FMT_RGBA8888) || (_format == MM_UTIL_IMG_FMT_BGRX8888)) {
		_bool = FALSE;
	} else {
		_bool = TRUE;
	}

	return _bool;
}

static gboolean __mm_select_rotate_plugin(mm_util_img_format _format, unsigned int width, unsigned int height, mm_util_img_rotate_type angle)
{
	debug_log("_format: %d (angle: %d)", _format, angle);

	if ((_format == MM_UTIL_IMG_FMT_YUV420) || (_format == MM_UTIL_IMG_FMT_I420) || (_format == MM_UTIL_IMG_FMT_NV12)
		||(_format == MM_UTIL_IMG_FMT_RGB888 ||_format == MM_UTIL_IMG_FMT_RGB565)) {
		return TRUE;
	}

	return FALSE;
}

static gboolean __mm_is_rgb_format(mm_util_img_format format)
{
	debug_log("format: %d", format);

	if ((format == MM_UTIL_IMG_FMT_RGB565) ||
		(format == MM_UTIL_IMG_FMT_RGB888) ||
		(format == MM_UTIL_IMG_FMT_ARGB8888) ||
		(format == MM_UTIL_IMG_FMT_BGRA8888) ||
		(format == MM_UTIL_IMG_FMT_RGBA8888) ||
		(format == MM_UTIL_IMG_FMT_BGRX8888)) {
		return TRUE;
	}

	return FALSE;
}

int __mm_util_get_buffer_size(mm_util_img_format format, unsigned int width, unsigned int height, unsigned int *imgsize)
{
	int ret = MM_ERROR_NONE;
	unsigned char x_chroma_shift = 0;
	unsigned char y_chroma_shift = 0;
	int size, w2, h2, size2;
	int stride, stride2;

	TTRACE_BEGIN("MM_UTILITY:IMGP:GET_SIZE");

	if (!imgsize) {
		debug_error("imgsize can't be null");
		TTRACE_END();
		return MM_ERROR_IMAGE_FILEOPEN;
	}

	*imgsize = 0;

	if (check_valid_picture_size(width, height) < 0) {
		debug_error("invalid width and height");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	switch (format)
	{
		case MM_UTIL_IMG_FMT_I420:
		case MM_UTIL_IMG_FMT_YUV420:
			x_chroma_shift = 1;
			y_chroma_shift = 1;
			stride = MM_UTIL_ROUND_UP_4 (width);
			h2 = ROUND_UP_X (height, x_chroma_shift);
			size = stride * h2;
			w2 = DIV_ROUND_UP_X (width, x_chroma_shift);
			stride2 = MM_UTIL_ROUND_UP_4 (w2);
			h2 = DIV_ROUND_UP_X (height, y_chroma_shift);
			size2 = stride2 * h2;
			*imgsize = size + 2 * size2;
			break;
		case MM_UTIL_IMG_FMT_YUV422:
		case MM_UTIL_IMG_FMT_YUYV:
		case MM_UTIL_IMG_FMT_UYVY:
		case MM_UTIL_IMG_FMT_NV16:
		case MM_UTIL_IMG_FMT_NV61:
			stride = MM_UTIL_ROUND_UP_4 (width) * 2;
			size = stride * height;
			*imgsize = size;
			break;

		case MM_UTIL_IMG_FMT_RGB565:
			stride = MM_UTIL_ROUND_UP_4 (width) * 2;
			size = stride * MM_UTIL_ROUND_UP_2(height);
			*imgsize = size;
			break;

		case MM_UTIL_IMG_FMT_RGB888:
			stride = MM_UTIL_ROUND_UP_4 (width) * 3;
			size = stride * MM_UTIL_ROUND_UP_2(height);
			*imgsize = size;
			break;

		case MM_UTIL_IMG_FMT_ARGB8888:
		case MM_UTIL_IMG_FMT_BGRA8888:
		case MM_UTIL_IMG_FMT_RGBA8888:
		case MM_UTIL_IMG_FMT_BGRX8888:
			stride = width * 4;
			size = stride * MM_UTIL_ROUND_UP_2(height);
			*imgsize = size;
			break;


		case MM_UTIL_IMG_FMT_NV12:
		case MM_UTIL_IMG_FMT_NV12_TILED:
			x_chroma_shift = 1;
			y_chroma_shift = 1;
			stride = MM_UTIL_ROUND_UP_4 (width);
			h2 = ROUND_UP_X (height, y_chroma_shift);
			size = stride * h2;
			w2 = 2 * DIV_ROUND_UP_X (width, x_chroma_shift);
			stride2 = MM_UTIL_ROUND_UP_4 (w2);
			h2 = DIV_ROUND_UP_X (height, y_chroma_shift);
			size2 = stride2 * h2;
			*imgsize = size + size2;
			break;

		default:
			debug_error("Not supported format");
			TTRACE_END();
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	debug_log("format: %d, *imgsize: %d\n", format, *imgsize);

	TTRACE_END();
	return ret;
}

static int __mm_confirm_dst_width_height(unsigned int src_width, unsigned int src_height, unsigned int *dst_width, unsigned int *dst_height, mm_util_img_rotate_type angle)
{
	int ret = MM_ERROR_NONE;

	if (!dst_width || !dst_height) {
		debug_error("[%s][%05d] dst_width || dst_height Buffer is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	switch(angle) {
		case MM_UTIL_ROTATE_0:
		case MM_UTIL_ROTATE_180:
		case MM_UTIL_ROTATE_FLIP_HORZ:
		case MM_UTIL_ROTATE_FLIP_VERT:
			if(*dst_width != src_width) {
				debug_log("*dst_width: %d", *dst_width);
				*dst_width = src_width;
				debug_log("#Confirmed# *dst_width: %d", *dst_width);
			}
			if(*dst_height != src_height) {
				debug_log("*dst_height: %d", *dst_height);
				*dst_height = src_height;
				debug_log("#Confirmed# *dst_height: %d", *dst_height);
			}
			break;
		case MM_UTIL_ROTATE_90:
		case MM_UTIL_ROTATE_270:
			if(*dst_width != src_height) {
				debug_log("*dst_width: %d", *dst_width);
				*dst_width = src_height;
				debug_log("#Confirmed# *dst_width: %d", *dst_width);
			}
			if(*dst_height != src_width) {
				debug_log("*dst_height: %d", *dst_height);
				*dst_height = src_width;
				debug_log("#Confirmed# *dst_height: %d", *dst_height);
			}
			break;

		default:
			debug_error("Not supported rotate value");
			return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	return ret;
}

static int __mm_set_format_label(imgp_info_s * _imgp_info_s, mm_util_img_format src_format, mm_util_img_format dst_format)
{
	int ret = MM_ERROR_NONE;
	char *src_fmt_lable = NULL;
	char *dst_fmt_lable = NULL;
	if(_imgp_info_s == NULL) {
		debug_error("_imgp_info_s: 0x%2x", _imgp_info_s);
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	switch(src_format) {
		case MM_UTIL_IMG_FMT_YUV420:
			src_fmt_lable = (char *)"YV12";
			break;
		case MM_UTIL_IMG_FMT_YUV422:
			src_fmt_lable = (char *)"Y42B";
			break;
		case MM_UTIL_IMG_FMT_I420:
			src_fmt_lable = (char *)"I420";
			break;
		case MM_UTIL_IMG_FMT_NV12:
			src_fmt_lable = (char *)"NV12";
			break;
		case MM_UTIL_IMG_FMT_UYVY:
			src_fmt_lable = (char *)"UYVY";
			break;
		case MM_UTIL_IMG_FMT_YUYV:
			src_fmt_lable = (char *)"YUYV";
			break;
		case MM_UTIL_IMG_FMT_RGB565:
			src_fmt_lable = (char *)"RGB565";
			break;
		case MM_UTIL_IMG_FMT_RGB888:
			src_fmt_lable = (char *)"RGB888";
			break;
		case MM_UTIL_IMG_FMT_ARGB8888:
			src_fmt_lable = (char *)"ARGB8888";
			break;
		case MM_UTIL_IMG_FMT_BGRA8888:
			src_fmt_lable = (char *)"BGRA8888";
			break;
		case MM_UTIL_IMG_FMT_RGBA8888:
			src_fmt_lable = (char *)"RGBA8888";
			break;
		case MM_UTIL_IMG_FMT_BGRX8888:
			src_fmt_lable = (char *)"BGRX";
			break;
		default:
			debug_log("[%d] Not supported format", src_fmt_lable);
			break;
	}

	switch(dst_format) {
		case MM_UTIL_IMG_FMT_YUV420:
			dst_fmt_lable = (char *)"YV12";
			break;
		case MM_UTIL_IMG_FMT_YUV422:
			dst_fmt_lable = (char *)"Y42B";
			break;
		case MM_UTIL_IMG_FMT_I420:
			dst_fmt_lable = (char *)"I420";
			break;
		case MM_UTIL_IMG_FMT_NV12:
			dst_fmt_lable = (char *)"NV12";
			break;
		case MM_UTIL_IMG_FMT_UYVY:
			dst_fmt_lable = (char *)"UYVY";
			break;
		case MM_UTIL_IMG_FMT_YUYV:
			dst_fmt_lable = (char *)"YUYV";
			break;
		case MM_UTIL_IMG_FMT_RGB565:
			dst_fmt_lable = (char *)"RGB565";
			break;
		case MM_UTIL_IMG_FMT_RGB888:
			dst_fmt_lable = (char *)"RGB888";
			break;
		case MM_UTIL_IMG_FMT_ARGB8888:
			dst_fmt_lable = (char *)"ARGB8888";
			break;
		case MM_UTIL_IMG_FMT_BGRA8888:
			dst_fmt_lable = (char *)"BGRA8888";
			break;
		case MM_UTIL_IMG_FMT_RGBA8888:
			dst_fmt_lable = (char *)"RGBA8888";
			break;
		case MM_UTIL_IMG_FMT_BGRX8888:
			dst_fmt_lable = (char *)"BGRX";
			break;
		default:
			debug_error("[%d] Not supported format", dst_format);
			break;
	}

	if(src_fmt_lable && dst_fmt_lable) {
		debug_log("src_fmt_lable: %s dst_fmt_lable: %s", src_fmt_lable, dst_fmt_lable);
		_imgp_info_s->input_format_label = (char*)malloc(strlen(src_fmt_lable) + 1);
		if(_imgp_info_s->input_format_label == NULL) {
			debug_error("[input] input_format_label is null");
			return MM_ERROR_IMAGE_NO_FREE_SPACE;
		}
		memset(_imgp_info_s->input_format_label, 0, strlen(src_fmt_lable) + 1);
		strncpy(_imgp_info_s->input_format_label, src_fmt_lable, strlen(src_fmt_lable));

		_imgp_info_s->output_format_label = (char*)malloc(strlen(dst_fmt_lable) + 1);
		if(_imgp_info_s->output_format_label == NULL) {
			debug_error("[input] input_format_label is null");
			IMGP_FREE(_imgp_info_s->input_format_label);
			return MM_ERROR_IMAGE_NO_FREE_SPACE;
		}
		memset(_imgp_info_s->output_format_label, 0, strlen(dst_fmt_lable) + 1);
		strncpy(_imgp_info_s->output_format_label, dst_fmt_lable, strlen(dst_fmt_lable));

		debug_log("input_format_label: %s output_format_label: %s", _imgp_info_s->input_format_label, _imgp_info_s->output_format_label);
	}else {
		debug_error("[error] src_fmt_lable && dst_fmt_lable");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	return ret;
}

static int __mm_set_imgp_info_s(imgp_info_s * _imgp_info_s, mm_util_img_format src_format, unsigned int src_width, unsigned int src_height, mm_util_img_format dst_format, unsigned int dst_width, unsigned int dst_height, mm_util_img_rotate_type angle)
{
	int ret = MM_ERROR_NONE;

	if(_imgp_info_s == NULL) {
		debug_error("_imgp_info_s is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	ret = __mm_set_format_label(_imgp_info_s, src_format, dst_format);
	if(ret != MM_ERROR_NONE) {
		debug_error("[input] mm_set_format_label error");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	_imgp_info_s->src_format = src_format;
	_imgp_info_s->src_width = src_width;
	_imgp_info_s->src_height= src_height;

	_imgp_info_s->dst_format = dst_format;
	_imgp_info_s->dst_width = dst_width;
	_imgp_info_s->dst_height = dst_height;
	_imgp_info_s->buffer_size = 0;
	_imgp_info_s->angle= angle;

	debug_log("[input] format label: %s width: %d height: %d [output] format label: %s width: %d height: %d rotation_value: %d",
	_imgp_info_s->input_format_label, _imgp_info_s->src_width, _imgp_info_s->src_height,
	_imgp_info_s->output_format_label, _imgp_info_s->dst_width, _imgp_info_s->dst_height, _imgp_info_s->angle);

	return ret;
}

static GModule * __mm_util_imgp_initialize(imgp_plugin_type_e _imgp_plugin_type_e)
{
	GModule *module = NULL;
	debug_fenter();

	if( _imgp_plugin_type_e == IMGP_NEON ) {
		module = g_module_open(PATH_NEON_LIB, G_MODULE_BIND_LAZY);
	}else if( _imgp_plugin_type_e == IMGP_GSTCS) {
		module = g_module_open(PATH_GSTCS_LIB, G_MODULE_BIND_LAZY);
	}

	if( module == NULL ) {
		debug_error("%s | %s module open failed", PATH_NEON_LIB, PATH_GSTCS_LIB);
		return NULL;
	}
	debug_log("module: %p, g_module_name: %s", module, g_module_name (module));

	return module;
}

static IMGPInfoFunc __mm_util_imgp_process(GModule *module)
{
	IMGPInfoFunc mm_util_imgp_func = NULL;
	debug_fenter();

	if(module == NULL) {
		debug_error("module is NULL");
		return NULL;
	}

	g_module_symbol(module, IMGP_FUNC_NAME, (gpointer*)&mm_util_imgp_func);
	debug_log("mm_util_imgp_func: %p", mm_util_imgp_func);

	return mm_util_imgp_func;
}

static int _mm_util_transform_packet_finalize_callback(media_packet_h packet, int err, void* userdata)
{
	debug_log("==> finalize callback func is called [%d] \n", err);
	return MEDIA_PACKET_FINALIZE;
}

static int __mm_util_imgp_finalize(GModule *module, imgp_info_s *_imgp_info_s)
{
	int ret = MM_ERROR_NONE;

	if(module) {
		debug_log("module : %p", module);
		g_module_close( module );
		debug_log("#End g_module_close#");
		module = NULL;
	}else {
		debug_error("#module is NULL#");
		ret = MM_ERROR_IMAGE_INVALID_VALUE;
	}

	IMGP_FREE(_imgp_info_s->input_format_label);
	IMGP_FREE(_imgp_info_s->output_format_label);
	IMGP_FREE(_imgp_info_s);

	return ret;
}

static int __mm_util_crop_rgba32(const unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int crop_dest_width, unsigned int crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;
	unsigned int idx = 0;
	int src_bytesperline = src_width * 4;
	int dst_bytesperline = crop_dest_width * 4;

	debug_log("[Input] src: 0x%2x src, src_width: %d src_height: %d src_format: %d crop_start_x: %d crop_start_y: %d crop_dest_width: %d crop_dest_height: %d\n",
	src, src_width, src_height, src_format, crop_start_x, crop_start_y, crop_dest_width, crop_dest_height);
	src += crop_start_y * src_bytesperline + 4 * crop_start_x;

	for (idx = 0; idx < crop_dest_height; idx++) {
		memcpy(dst, src, dst_bytesperline);
		src += src_bytesperline;
		dst += dst_bytesperline;
	}

	return ret;
}

static int __mm_util_crop_rgb888(const unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int crop_dest_width, unsigned int crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;
	unsigned int idx = 0;
	int src_bytesperline = src_width * 3;
	int dst_bytesperline = crop_dest_width * 3;

	debug_log("[Input] src: 0x%2x src, src_width: %d src_height: %d src_format: %d crop_start_x: %d crop_start_y: %d crop_dest_width: %d crop_dest_height: %d\n",
	src, src_width, src_height, src_format, crop_start_x, crop_start_y, crop_dest_width, crop_dest_height);
	src += crop_start_y * src_bytesperline + 3 * crop_start_x;

	for (idx = 0; idx < crop_dest_height; idx++) {
		memcpy(dst, src, dst_bytesperline);
		src += src_bytesperline;
		dst += dst_bytesperline;
	}

	return ret;
}

static int __mm_util_crop_rgb565(const unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int crop_dest_width, unsigned int crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;
	unsigned int idx = 0;
	int src_bytesperline = src_width * 2;
	int dst_bytesperline = crop_dest_width * 2;

	debug_log("[Input] src: 0x%2x src, src_width: %d src_height: %d src_format: %d crop_start_x: %d crop_start_y: %d crop_dest_width: %d crop_dest_height: %d\n",
	src, src_width, src_height, src_format, crop_start_x, crop_start_y, crop_dest_width, crop_dest_height);
	src += crop_start_y * src_bytesperline + 2 * crop_start_x;

	for (idx = 0; idx < crop_dest_height; idx++) {
		memcpy(dst, src, dst_bytesperline);
		src += src_bytesperline;
		dst += dst_bytesperline;
	}

	return ret;
}

static int __mm_util_crop_yuv420(const unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int crop_dest_width, unsigned int crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;
	unsigned int idx = 0;
	int start_x = crop_start_x;
	int start_y = crop_start_y;
	debug_log("[Input] src: 0x%2x src, src_width: %d src_height: %d src_format: %d crop_start_x: %d crop_start_y: %d crop_dest_width: %d crop_dest_height: %d\n",
	src, src_width, src_height, src_format, crop_start_x, crop_start_y, crop_dest_width, crop_dest_height);
	const unsigned char *_src = src + start_y * src_width + start_x;

	/* Y */
	for (idx = 0; idx < crop_dest_height; idx++) {
		memcpy(dst, _src, crop_dest_width);
		_src += src_width;
		dst += crop_dest_width;
	}

	/* U */
	_src = src + src_height * src_width + (start_y / 2) * src_width / 2 + start_x / 2;
	for (idx = 0; idx < crop_dest_height / 2; idx++) {
		memcpy(dst, _src, crop_dest_width / 2);
		_src += src_width / 2;
		dst += crop_dest_width / 2;
	}

	/* V */
	_src = src + src_height * src_width * 5 / 4 + (start_y / 2) * src_width / 2 + start_x / 2;
	for (idx = 0; idx < crop_dest_height / 2; idx++) {
		memcpy(dst, _src, crop_dest_width / 2);
		_src += src_width / 2;
		dst += crop_dest_width / 2;
	}

	return ret;
}

static bool __mm_util_check_resolution(unsigned int width, unsigned int height)
{
	if(width == 0)
	{
		debug_error("invalid width [%d]", width);
		return FALSE;
	}

	if(height == 0)
	{
		debug_error("invalid height [%d]", height);
		return FALSE;
	}

	return TRUE;
}

static int __mm_util_handle_init(mm_util_s *handle)
{
	int ret = MM_ERROR_NONE;

	if (!handle) {
		debug_error("[ERROR] - handle");
		return MM_ERROR_IMAGE_INTERNAL;
	}

	/* private values init */
	handle->src = NULL;
	handle->dst = NULL;
	handle->drm_fd = -1;
	handle->dst_format = MM_UTIL_IMG_FMT_NUM;
	handle->dst_format_mime = -1;
	handle->src_buf_idx = 0;
	handle->dst_buf_idx = 0;
	handle->dst_rotation = MM_UTIL_ROTATION_NONE;

	handle->start_x = -1;
	handle->start_y = -1;
	handle->src_width = 0;
	handle->src_height = 0;
	handle->dst_width = 0;
	handle->dst_height = 0;
	handle->is_completed = FALSE;
	handle->is_finish = FALSE;

	handle->set_convert = FALSE;
	handle->set_crop = FALSE;
	handle->set_resize = FALSE;
	handle->set_rotate = FALSE;

	return ret;
}

static int __mm_util_handle_refresh(mm_util_s *handle)
{
	int ret = MM_ERROR_NONE;

	if (!handle) {
		debug_error("[ERROR] - handle");
		return MM_ERROR_IMAGE_INTERNAL;
	}

	/* restore original settings */
	if (handle->set_rotate && (handle->set_crop || handle->set_resize)) {
		if ((handle->dst_rotation == MM_UTIL_ROTATION_90) || (handle->dst_rotation == MM_UTIL_ROTATION_270)) {
			unsigned int temp = 0;
			temp = handle->dst_width;
			handle->dst_width = handle->dst_height;
			handle->dst_height = temp;
		}
	}

	handle->is_completed = FALSE;

	handle->dst_packet = NULL;

	return ret;
}

static media_format_mimetype_e __mm_util_mapping_imgp_format_to_mime(mm_util_img_format format)
{
	media_format_mimetype_e mimetype = -1;

	switch(format) {
		case MM_UTIL_IMG_FMT_NV12 :
			mimetype = MEDIA_FORMAT_NV12;
			break;
		case MM_UTIL_IMG_FMT_NV16 :
			mimetype = MEDIA_FORMAT_NV16;
			break;
		case MM_UTIL_IMG_FMT_YUYV :
			mimetype = MEDIA_FORMAT_YUYV;
			break;
		case MM_UTIL_IMG_FMT_UYVY :
			mimetype = MEDIA_FORMAT_UYVY;
			break;
		case MM_UTIL_IMG_FMT_YUV422 :
			mimetype = MEDIA_FORMAT_422P;
			break;
		case MM_UTIL_IMG_FMT_I420 :
			mimetype = MEDIA_FORMAT_I420;
			break;
		case MM_UTIL_IMG_FMT_YUV420 :
			mimetype = MEDIA_FORMAT_YV12;
			break;
		case MM_UTIL_IMG_FMT_RGB565 :
			mimetype = MEDIA_FORMAT_RGB565;
			break;
		case MM_UTIL_IMG_FMT_RGB888 :
			mimetype = MEDIA_FORMAT_RGB888;
			break;
		case MM_UTIL_IMG_FMT_RGBA8888 :
			mimetype = MEDIA_FORMAT_RGBA;
			break;
		case MM_UTIL_IMG_FMT_ARGB8888 :
			mimetype = MEDIA_FORMAT_ARGB;
			break;
		case MM_UTIL_IMG_FMT_NV12_TILED :
			mimetype = MEDIA_FORMAT_NV12T;
			break;
		default:
			mimetype = -1;
			debug_error("Not Supported Format [%d]", format);
			break;
	}

	debug_log("imgp fmt: %d mimetype fmt: %d", format, mimetype);

	return mimetype;
}

static mm_util_img_format __mm_util_mapping_mime_format_to_imgp(media_format_mimetype_e mimetype)
{
	mm_util_img_format format = -1;

	switch(mimetype) {
		case MEDIA_FORMAT_NV12 :
			format = MM_UTIL_IMG_FMT_NV12;
			break;
		case MEDIA_FORMAT_NV16 :
			format = MM_UTIL_IMG_FMT_NV16;
			break;
		case MEDIA_FORMAT_YUYV :
			format = MM_UTIL_IMG_FMT_YUYV;
			break;
		case MEDIA_FORMAT_UYVY :
			format = MM_UTIL_IMG_FMT_UYVY;
			break;
		case MEDIA_FORMAT_422P :
			format = MM_UTIL_IMG_FMT_YUV422;
			break;
		case MEDIA_FORMAT_I420 :
			format = MM_UTIL_IMG_FMT_I420;
			break;
		case MEDIA_FORMAT_YV12 :
			format = MM_UTIL_IMG_FMT_YUV420;
			break;
		case MEDIA_FORMAT_RGB565 :
			format = MM_UTIL_IMG_FMT_RGB565;
			break;
		case MEDIA_FORMAT_RGB888 :
			format = MM_UTIL_IMG_FMT_RGB888;
			break;
		case MEDIA_FORMAT_RGBA :
			format = MM_UTIL_IMG_FMT_RGBA8888;
			break;
		case MEDIA_FORMAT_ARGB :
			format = MM_UTIL_IMG_FMT_ARGB8888;
			break;
		case MEDIA_FORMAT_NV12T :
			format = MM_UTIL_IMG_FMT_NV12_TILED;
			break;
		default:
			format = -1;
			debug_error("Not Supported Format [%d]", mimetype);
			break;
	}

	debug_log("mimetype: %d imgp fmt: %d", mimetype, format);

	return format;
}

gpointer _mm_util_thread_repeate(gpointer data)
{
	mm_util_s* handle = (mm_util_s*) data;
	int ret = MM_ERROR_NONE;
	gint64 end_time = 0;

	if (!handle) {
		debug_error("[ERROR] - handle");
		return NULL;
	}

	while (!handle->is_finish) {
		end_time = g_get_monotonic_time() + 1 * G_TIME_SPAN_SECOND;
		debug_log("waiting...");
		g_mutex_lock (&(handle->thread_mutex));
		g_cond_wait_until(&(handle->thread_cond), &(handle->thread_mutex), end_time);
		debug_log("<=== get run transform thread signal");
		g_mutex_unlock (&(handle->thread_mutex));

		if (handle->is_finish) {
			debug_log("exit loop");
			break;
		}

		media_packet_h pop_data = (media_packet_h) g_async_queue_try_pop(handle->queue);

		if(!pop_data) {
			debug_error("[NULL] Queue data");
		} else {
			ret = __mm_util_transform_exec(handle, pop_data); /* Need to block */
			if(ret == MM_ERROR_NONE) {
				debug_log("Success - transform_exec");
			} else{
				debug_error("Error - transform_exec");
			}
			if(handle->_util_cb->completed_cb) {
				debug_log("completed_cb");
				handle->_util_cb->completed_cb(&handle->dst_packet, ret, handle->_util_cb->user_data);
				debug_log("completed_cb %p", &handle->dst);
			}

			__mm_util_handle_refresh(handle);
		}
	}

	debug_log("exit thread");

	return NULL;
}

static int __mm_util_create_thread(mm_util_s *handle)
{
	int ret = MM_ERROR_NONE;

	if (!handle) {
		debug_error("[ERROR] - handle");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	g_mutex_init(&(handle->thread_mutex));

	/*These are a communicator for thread*/
	if(!handle->queue) {
		handle->queue = g_async_queue_new();
	} else {
		debug_error("ERROR - async queue is already created");
	}

	g_cond_init(&(handle->thread_cond));

	/*create threads*/
	handle->thread = g_thread_new("transform_thread", (GThreadFunc)_mm_util_thread_repeate, (gpointer)handle);
	if(!handle->thread) {
		debug_error("ERROR - create thread");
		return MM_ERROR_IMAGE_INTERNAL;
	}

	return ret;
}

static int __mm_util_processing(mm_util_s *handle)
{
	int ret = MM_ERROR_NONE;
	unsigned char *dst_buf[4] = {NULL,};
	unsigned int dst_buf_size = 0;
	unsigned int src_width = 0, src_height = 0;
	mm_util_img_format src_format;
	unsigned int src_index = 0, dst_index = 0;

	if(handle == NULL) {
		debug_error ("Invalid arguments [tag null]");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if(handle->src_packet == NULL) {
		debug_error ("[src] media_packet_h");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if(handle->dst_packet == NULL) {
		debug_error ("[dst] media_packet_h");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if(handle->src_buf_size) {
		handle->src = NULL;
		if(media_packet_get_buffer_data_ptr(handle->src_packet, &handle->src) != MM_ERROR_NONE) {
			debug_error ("[src] media_packet_get_extra");
			IMGP_FREE(handle->src);
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}
		debug_log("src buffer pointer: %p", handle->src);
	}

	if(handle->dst_buf_size) {
		handle->dst = NULL;
		if(media_packet_get_buffer_data_ptr(handle->dst_packet, &handle->dst) != MM_ERROR_NONE) {
			IMGP_FREE(handle->src);
			IMGP_FREE(handle->dst);
			debug_error ("[dst] media_packet_get_extra");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}
	}

	debug_log("src: %p, dst: %p", handle->src, handle->dst);

	dst_buf[src_index] = g_malloc(handle->src_buf_size);
	src_width = handle->src_width;
	src_height = handle->src_height;
	src_format = handle->src_format;
	if (dst_buf[src_index] == NULL) {
		debug_error ("[multi func] memory allocation error");
		IMGP_FREE(handle->src);
		IMGP_FREE(handle->dst);
		return MM_ERROR_IMAGE_INTERNAL;
	}
	memcpy(dst_buf[src_index], handle->src, handle->src_buf_size);
	if (handle->set_crop) {
		dst_index++;
		mm_util_get_image_size(src_format, handle->dst_width, handle->dst_height, &dst_buf_size);
		dst_buf[dst_index] = g_malloc(dst_buf_size);
		if (dst_buf[dst_index] == NULL) {
			debug_error ("[multi func] memory allocation error");
			IMGP_FREE(handle->src);
			IMGP_FREE(handle->dst);
			__mm_destroy_temp_buffer(dst_buf);
			return MM_ERROR_IMAGE_INTERNAL;
		}
		ret = mm_util_crop_image(dst_buf[src_index], src_width, src_height, src_format,
		handle->start_x, handle->start_y, &handle->dst_width, &handle->dst_height, dst_buf[dst_index]);
		if (ret != MM_ERROR_NONE) {
			IMGP_FREE(handle->src);
			IMGP_FREE(handle->dst);
			__mm_destroy_temp_buffer(dst_buf);
			debug_error("mm_util_crop_image failed");
			return ret;
		}
		src_index = dst_index;
		src_width = handle->dst_width;
		src_height = handle->dst_height;
	} else if (handle->set_resize) {
		dst_index++;
		mm_util_get_image_size(src_format, handle->dst_width, handle->dst_height, &dst_buf_size);
		dst_buf[dst_index] = g_malloc(dst_buf_size);
		if (dst_buf[dst_index] == NULL) {
			debug_error ("[multi func] memory allocation error");
			IMGP_FREE(handle->src);
			IMGP_FREE(handle->dst);
			__mm_destroy_temp_buffer(dst_buf);
			return MM_ERROR_IMAGE_INTERNAL;
		}
		ret = mm_util_resize_image(dst_buf[src_index], src_width, src_height,src_format, dst_buf[dst_index], &handle->dst_width, &handle->dst_height);
		if (ret != MM_ERROR_NONE) {
			IMGP_FREE(handle->src);
			IMGP_FREE(handle->dst);
			__mm_destroy_temp_buffer(dst_buf);
			debug_error("mm_util_resize_image failed");
			return ret;
		}
		src_index = dst_index;
		src_width = handle->dst_width;
		src_height = handle->dst_height;
	}

	if (handle->set_convert) {
		dst_index++;
		mm_util_get_image_size(handle->dst_format, handle->dst_width, handle->dst_height, &dst_buf_size);
		dst_buf[dst_index] = g_malloc(dst_buf_size);
		if (dst_buf[dst_index] == NULL) {
			debug_error ("[multi func] memory allocation error");
			IMGP_FREE(handle->src);
			IMGP_FREE(handle->dst);
			__mm_destroy_temp_buffer(dst_buf);
			return MM_ERROR_IMAGE_INTERNAL;
		}
		ret = mm_util_convert_colorspace(dst_buf[src_index], src_width, src_height, src_format, dst_buf[dst_index], handle->dst_format);
		if (ret != MM_ERROR_NONE) {
			IMGP_FREE(handle->src);
			IMGP_FREE(handle->dst);
			__mm_destroy_temp_buffer(dst_buf);
			debug_error("mm_util_convert_colorspace failed");
			return ret;
		}
		src_index = dst_index;
		src_format = handle->dst_format;
	}

	if (handle->set_rotate) {
		dst_index++;
		if(handle->set_resize || handle->set_crop) {
			unsigned int temp_swap = 0;
			switch(handle->dst_rotation) {
				case  MM_UTIL_ROTATION_90:
				case MM_UTIL_ROTATION_270:
					temp_swap = handle->dst_width;
					handle->dst_width  = handle->dst_height;
					handle->dst_height = temp_swap;
					break;
				default:
					break;
			}
		}
		mm_util_get_image_size(src_format, handle->dst_width, handle->dst_height, &dst_buf_size);
		dst_buf[dst_index] = g_malloc(dst_buf_size);
		if (dst_buf[dst_index] == NULL) {
			debug_error ("[multi func] memory allocation error");
			IMGP_FREE(handle->src);
			IMGP_FREE(handle->dst);
			__mm_destroy_temp_buffer(dst_buf);
			return MM_ERROR_IMAGE_INTERNAL;
		}
		ret = mm_util_rotate_image(dst_buf[src_index], src_width, src_height, src_format, dst_buf[dst_index], &handle->dst_width, &handle->dst_height, handle->dst_rotation);
		if (ret != MM_ERROR_NONE) {
			IMGP_FREE(handle->src);
			IMGP_FREE(handle->dst);
			__mm_destroy_temp_buffer(dst_buf);
			debug_error("mm_util_rotate_image failed");
			return ret;
		}
		src_index = dst_index;
		src_width = handle->dst_width;
		src_height = handle->dst_height;
	}

	if (dst_buf[dst_index] != NULL && dst_buf_size != 0) {
		memcpy(handle->dst, dst_buf[dst_index], dst_buf_size);
		handle->dst_buf_size = dst_buf_size;
	}
	__mm_destroy_temp_buffer(dst_buf);

	debug_error("mm_util_processing was finished");

	return ret;
}

static int __mm_util_transform_exec(mm_util_s * handle, media_packet_h src_packet)
{
	int ret = MM_ERROR_NONE;
	media_format_h src_fmt;
	media_format_h dst_fmt;
	media_format_mimetype_e src_mimetype;
	int src_width, src_height, src_avg_bps, src_max_bps;
	unsigned int dst_width = 0, dst_height = 0;
	uint64_t size = 0;

	if(media_packet_get_format(src_packet, &src_fmt) != MM_ERROR_NONE) {
		debug_error("Imedia_packet_get_format)");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if(media_format_get_video_info(src_fmt, &src_mimetype, &src_width, &src_height, &src_avg_bps, &src_max_bps) == MEDIA_FORMAT_ERROR_NONE) {
		debug_log("[Fotmat: %d] W x H : %d x %d", src_mimetype, src_width, src_height);
	}

	if(__mm_util_check_resolution(src_width, src_height)) {
		/* src */
		handle->src_packet = src_packet;
		debug_log("src_packet: %p handle->src_packet: %p 0x%2x [W X H] %d X %d", src_packet, handle->src_packet, src_fmt, src_width, src_height);
		if(handle->src_packet) {
			handle->src_format = __mm_util_mapping_mime_format_to_imgp(src_mimetype);
			handle->src_width = src_width;
			handle->src_height = src_height;
		} else {
			debug_error("[Error] handle->src");
			media_format_unref(src_fmt);
			return MM_ERROR_IMAGEHANDLE_NOT_INITIALIZED;
		}

		if(media_packet_get_buffer_size(handle->src_packet, &size) == MM_ERROR_NONE) {
			handle->src_buf_size = (guint)size;
			debug_log("src buffer(%p) %d size: %d", handle->src_packet, handle->src_packet, handle->src_buf_size);
		} else {
			debug_error("Error buffer size");
		}

		if(handle->dst_format == MM_UTIL_IMG_FMT_NUM) {
			debug_log("dst format is changed");
			handle->dst_format = handle->src_format;
			handle->dst_format_mime = src_mimetype;
		}

		debug_log("src: %p handle->src_packet: %p (%d),(%d X %d)", src_packet, handle->src_packet, handle->src_packet, handle->src_width, handle->src_height);
		if (handle->set_rotate) {
			if ((handle->set_crop) || (handle->set_resize)) {
				switch(handle->dst_rotation) {
					case MM_UTIL_ROTATION_90:
					case MM_UTIL_ROTATION_270:
						dst_width = handle->dst_height;
						dst_height = handle->dst_width;
						break;
					default:
						dst_width = handle->dst_width;
						dst_height = handle->dst_height;
						break;
				}
			} else {
				switch(handle->dst_rotation) {
					case MM_UTIL_ROTATION_90:
					case MM_UTIL_ROTATION_270:
						dst_width = handle->dst_width  = handle->src_height;
						dst_height = handle->dst_height = handle->src_width;
						break;
					case MM_UTIL_ROTATION_NONE:
					case MM_UTIL_ROTATION_180:
					case MM_UTIL_ROTATION_FLIP_HORZ:
					case MM_UTIL_ROTATION_FLIP_VERT:
						dst_width = handle->dst_width  = handle->src_width;
						dst_height = handle->dst_height = handle->src_height;
						break;
					default:
						debug_error("[Error] Wrong dst_rotation");
						break;
				}
			}
		} else {
			if ((handle->set_crop) || (handle->set_resize)) {
				dst_width = handle->dst_width;
				dst_height = handle->dst_height;
			} else {
				dst_width = handle->dst_width  = handle->src_width;
				dst_height = handle->dst_height = handle->src_height;
			}
		}
		debug_log("dst (%d X %d)", dst_width, dst_height);
		if(media_format_make_writable(src_fmt, &dst_fmt) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(src_fmt);
			debug_error("[Error] Writable - dst format");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_set_video_mime(dst_fmt, handle->dst_format_mime) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(src_fmt);
			media_format_unref(dst_fmt);
			debug_error("[Error] Set - video mime");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_set_video_width(dst_fmt, dst_width) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(src_fmt);
			media_format_unref(dst_fmt);
			debug_error("[Error] Set - video width");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_set_video_height(dst_fmt, dst_height) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(src_fmt);
			media_format_unref(dst_fmt);
			debug_error("[Error] Set - video height");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_set_video_avg_bps(dst_fmt, src_avg_bps) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(src_fmt);
			media_format_unref(dst_fmt);
			debug_error("[Error] Set - video avg bps");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_set_video_max_bps(dst_fmt, src_max_bps) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(src_fmt);
			media_format_unref(dst_fmt);
			debug_error("[Error] Set - video max bps");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_packet_create_alloc(dst_fmt, (media_packet_finalize_cb)_mm_util_transform_packet_finalize_callback, NULL, &handle->dst_packet) != MM_ERROR_NONE) {
			debug_error("[Error] Create allocation memory");
			media_format_unref(src_fmt);
			media_format_unref(dst_fmt);
			return MM_ERROR_IMAGE_INVALID_VALUE;
		} else {
			debug_log("Success - dst media packet");
			if(media_packet_get_buffer_size(handle->dst_packet, &size) != MM_ERROR_NONE) {
				debug_error("Imedia_packet_get_format)");
				media_format_unref(src_fmt);
				media_format_unref(dst_fmt);
				return MM_ERROR_IMAGE_INVALID_VALUE;
			}
			handle->dst_buf_size = (guint)size;
			debug_log("handle->src_packet: %p [%d] %d X %d (%d) => handle->dst_packet: %p [%d] %d X %d (%d)",
				handle->src_packet, handle->src_format, handle->src_width, handle->src_height, handle->src_buf_size,
				handle->dst_packet, handle->dst_format,dst_width, dst_height, handle->dst_buf_size);
		}
	}else {
		debug_error("%d %d", src_width, src_height);
		media_format_unref(src_fmt);
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	ret = __mm_util_processing(handle);

	if(ret != MM_ERROR_NONE) {
		debug_error("__mm_util_processing failed");
		IMGP_FREE(handle);
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	media_format_unref(src_fmt);
	media_format_unref(dst_fmt);

	return ret;
}

static int
_mm_util_handle_finalize(mm_util_s *handle)
{
	int ret = MM_ERROR_NONE;

	if (!handle) {
		debug_error("[ERROR] - handle");
		return MM_ERROR_IMAGE_INTERNAL;
	}

	/* g_thread_exit(handle->thread); */
	if(handle->thread) {
		handle->is_finish = TRUE;
		g_mutex_lock (&(handle->thread_mutex));
		g_cond_signal(&(handle->thread_cond));
		debug_log("===> send signal(finish) to transform_thread");
		g_mutex_unlock (&(handle->thread_mutex));

		g_thread_join(handle->thread);
	}

	if(handle->queue) {
		g_async_queue_unref(handle->queue);
		handle->queue = NULL;
	}

	g_mutex_clear(&(handle->thread_mutex));

	g_cond_clear(&(handle->thread_cond));

	debug_log("Success - Finalize Handle");

	return ret;
}

int mm_util_create(MMHandleType* MMHandle)
{
	int ret = MM_ERROR_NONE;

	TTRACE_BEGIN("MM_UTILITY:IMGP:CREATE");

	if(MMHandle == NULL) {
		debug_error ("Invalid arguments [tag null]");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	mm_util_s *handle = calloc(1,sizeof(mm_util_s));
	if (!handle) {
		debug_error("[ERROR] - _handle");
		ret = MM_ERROR_IMAGE_INTERNAL;
	}

	ret = __mm_util_handle_init (handle);
	if(ret != MM_ERROR_NONE) {
		debug_error("_mm_util_handle_init failed");
		IMGP_FREE(handle);
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	ret = __mm_util_create_thread(handle);
	if(ret != MM_ERROR_NONE) {
		debug_error("ERROR - Create thread");
		TTRACE_END();
		return ret;
	} else {
		debug_log("Success -__mm_util_create_thread");
	}

	*MMHandle = (MMHandleType)handle;

	TTRACE_END();
	return ret;
}

int mm_util_set_hardware_acceleration(MMHandleType MMHandle, bool mode)
{
	int ret = MM_ERROR_NONE;
	mm_util_s *handle = (mm_util_s *) MMHandle;

	TTRACE_BEGIN("MM_UTILITY:IMGP:SET_HARDWARE_ACCELERATION");

	if (!handle) {
		debug_error("[ERROR] - handle");
		TTRACE_END();
		return MM_ERROR_IMAGE_INTERNAL;
	}

	handle->hardware_acceleration = mode;

	TTRACE_END();
	return ret;
}

int mm_util_set_colorspace_convert(MMHandleType MMHandle, mm_util_img_format colorspace)
{
	int ret = MM_ERROR_NONE;
	mm_util_s *handle = (mm_util_s *) MMHandle;

	TTRACE_BEGIN("MM_UTILITY:IMGP:SET_HARDWARE_ACCELERATION");

	if (!handle) {
		debug_error("[ERROR] - handle");
		return MM_ERROR_IMAGE_INTERNAL;
	}

	handle->set_convert = TRUE;
	handle->dst_format = colorspace;
	handle->dst_format_mime = __mm_util_mapping_imgp_format_to_mime(colorspace);
	debug_log("imgp fmt: %d mimetype: %d", handle->dst_format, handle->dst_format_mime);

	TTRACE_END();
	return ret;
}

int mm_util_set_resolution(MMHandleType MMHandle, unsigned int width, unsigned int height)
{
	int ret = MM_ERROR_NONE;
	mm_util_s *handle = (mm_util_s *) MMHandle;

	TTRACE_BEGIN("MM_UTILITY:IMGP:SET_RESOLUTION");

	if (!handle) {
		debug_error("[ERROR] - handle");
		return MM_ERROR_IMAGE_INTERNAL;
	}

	handle->set_resize = TRUE;
	handle->dst_width = width;
	handle->dst_height = height;

	TTRACE_END();
	return ret;
}

int mm_util_set_rotation(MMHandleType MMHandle, mm_util_img_rotate_type rotation)
{
	int ret = MM_ERROR_NONE;
	mm_util_s *handle = (mm_util_s *) MMHandle;

	TTRACE_BEGIN("MM_UTILITY:IMGP:SET_ROTATION");

	if (!handle) {
		debug_error("[ERROR] - handle");
		TTRACE_END();
		return MM_ERROR_IMAGE_INTERNAL;
	}

	handle->set_rotate = TRUE;
	handle->dst_rotation = rotation;

	TTRACE_END();
	return ret;
}

int mm_util_set_crop_area(MMHandleType MMHandle, unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y)
{
	int ret = MM_ERROR_NONE;
	mm_util_s *handle = (mm_util_s *) MMHandle;

	TTRACE_BEGIN("MM_UTILITY:IMGP:SET_CROP_AREA");

	unsigned int dest_width = end_x -start_x;
	unsigned int dest_height = end_y - start_y;

	if (!handle) {
		debug_error("[ERROR] - handle");
		TTRACE_END();
		return MM_ERROR_IMAGE_INTERNAL;
	}

	handle->set_crop = TRUE;
	handle->start_x = start_x;
	handle->start_y = start_y;
	handle->dst_width = dest_width;
	handle->dst_height = dest_height;

	TTRACE_END();
	return ret;
}

int mm_util_transform(MMHandleType MMHandle, media_packet_h src_packet, mm_util_completed_callback completed_callback, void * user_data)
{
	int ret = MM_ERROR_NONE;
	mm_util_s *handle = (mm_util_s *) MMHandle;

	TTRACE_BEGIN("MM_UTILITY:IMGP:TRANSFORM");

	if (!handle) {
		debug_error("[ERROR] - handle");
		TTRACE_END();
		return MM_ERROR_IMAGE_INTERNAL;
	}

	if(!src_packet) {
		debug_error("[ERROR] - src_packet");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	} else {
		debug_log("src: %p", src_packet);
	}

	if(!completed_callback) {
		debug_error("[ERROR] - completed_callback");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	IMGP_FREE(handle->_util_cb);
	handle->_util_cb = (mm_util_cb_s *)malloc(sizeof(mm_util_cb_s));
	if(handle->_util_cb) {
		handle->_util_cb->completed_cb= completed_callback;
		handle->_util_cb->user_data = user_data;
	} else {
		debug_error("[ERROR] _util_cb_s");
	}

	if(handle->queue) {
		debug_log("g_async_queue_push");
		g_async_queue_push (handle->queue, GINT_TO_POINTER(src_packet));
		g_mutex_lock (&(handle->thread_mutex));
		g_cond_signal(&(handle->thread_cond));
		debug_log("===> send signal to transform_thread");
		g_mutex_unlock (&(handle->thread_mutex));
	}

	TTRACE_END();
	return ret;
}

int mm_util_transform_is_completed(MMHandleType MMHandle, bool *is_completed)
{
	int ret = MM_ERROR_NONE;

	mm_util_s *handle = (mm_util_s *) MMHandle;

	TTRACE_BEGIN("MM_UTILITY:IMGP:TRANSFORM_IS_COMPLETED");

	if (!handle) {
		debug_error("[ERROR] - handle");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if (!is_completed) {
		debug_error("[ERROR] - is_completed");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	*is_completed = handle->is_completed;
	debug_log("[Transform....] %d", *is_completed);

	TTRACE_END();
	return ret;
}

int mm_util_destroy(MMHandleType MMHandle)
{
	int ret = MM_ERROR_NONE;
	mm_util_s *handle = (mm_util_s*) MMHandle;

	TTRACE_BEGIN("MM_UTILITY:IMGP:DESTROY");

	if (!handle) {
		debug_error("[ERROR] - handle");
		TTRACE_END();
		return MM_ERROR_IMAGEHANDLE_NOT_INITIALIZED;
	}

	/* Close */
	if(_mm_util_handle_finalize(handle) != MM_ERROR_NONE) {
		debug_error("_mm_util_handle_finalize)");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	IMGP_FREE(handle->_util_cb);
	IMGP_FREE(handle);
	debug_log("Success - Destroy Handle");

	TTRACE_END();
	return ret;
}

EXPORT_API int mm_util_convert_colorspace(const unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format, unsigned char *dst, mm_util_img_format dst_format)
{
	int ret = MM_ERROR_NONE;
	unsigned char *output_buffer = NULL;
	unsigned int output_buffer_size = 0;

	TTRACE_BEGIN("MM_UTILITY:IMGP:CONVERT_COLORSPACE");

	debug_fenter();

	if (!src || !dst) {
		debug_error("invalid src or dst");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if ((src_format < MM_UTIL_IMG_FMT_YUV420) || (src_format > MM_UTIL_IMG_FMT_NUM) || (dst_format < MM_UTIL_IMG_FMT_YUV420) || (dst_format > MM_UTIL_IMG_FMT_NUM)) {
		debug_error("#ERROR# src_format : [%d] dst_format : [%d] value ", src_format, dst_format);
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if (__mm_cannot_convert_format(src_format, dst_format)) {
		debug_error("#ERROR# Cannot Support Image Format Convert");
		TTRACE_END();
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	debug_log("[src] 0x%2x (%d x %d) [dst] 0x%2x", src, src_width, src_height, dst);

	imgp_info_s* _imgp_info_s=(imgp_info_s*)g_malloc0(sizeof(imgp_info_s));
	if (_imgp_info_s == NULL) {
		debug_error("ERROR - alloc handle");
		TTRACE_END();
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}

	IMGPInfoFunc _mm_util_imgp_func = NULL;
	GModule *_module = NULL;
	imgp_plugin_type_e _imgp_plugin_type_e = 0;

	/* Initialize */
	if (__mm_select_convert_plugin(src_format, dst_format)) {
		_imgp_plugin_type_e = IMGP_NEON;
	} else {
		_imgp_plugin_type_e = IMGP_GSTCS;
	}

	debug_log("plugin type: %d", _imgp_plugin_type_e);

	_module = __mm_util_imgp_initialize(_imgp_plugin_type_e);
	debug_log("__mm_util_imgp_initialize: %p", _module);

	if(_module == NULL) { /* when IMGP_NEON is NULL */
		_imgp_plugin_type_e = IMGP_GSTCS;
		debug_log("You use %s module", PATH_GSTCS_LIB);
		_module = __mm_util_imgp_initialize(_imgp_plugin_type_e);
	}
	debug_log("mm_util_imgp_func: %p", _module);

	ret = __mm_set_imgp_info_s(_imgp_info_s, src_format, src_width, src_height, dst_format, src_width, src_height, MM_UTIL_ROTATE_0);
	if(ret != MM_ERROR_NONE) {
		debug_error("__mm_set_imgp_info_s failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}
	debug_log("Sucess __mm_set_imgp_info_s");

	ret = __mm_util_get_buffer_size(dst_format, src_width, src_height, &output_buffer_size);
	if (ret != MM_ERROR_NONE) {
		debug_error("__mm_set_imgp_info_s failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return ret;
	}

	output_buffer = (unsigned char*)malloc(output_buffer_size);
	if (output_buffer == NULL) {
		debug_error("malloc failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}
	debug_log("malloc outputbuffer: %p (%d)", output_buffer, output_buffer_size);

	/* image processing */
	_mm_util_imgp_func = __mm_util_imgp_process(_module);
	debug_log("Sucess __mm_util_imgp_process");

	if (_mm_util_imgp_func) {
		ret=_mm_util_imgp_func(_imgp_info_s, src, output_buffer, IMGP_CSC);
		if (ret != MM_ERROR_NONE)
		{
			debug_error("image processing failed");
			__mm_util_imgp_finalize(_module, _imgp_info_s);
			IMGP_FREE(output_buffer);
			TTRACE_END();
			return ret;
		}
	} else {
		debug_error("g_module_symbol failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		IMGP_FREE(output_buffer);
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if ((_imgp_info_s->dst_width != _imgp_info_s->output_stride || _imgp_info_s->dst_height != _imgp_info_s->output_elevation) && __mm_is_rgb_format(src_format)) {
		ret = mm_util_crop_image(output_buffer, _imgp_info_s->output_stride, _imgp_info_s->output_elevation, dst_format, 0, 0, &_imgp_info_s->dst_width, &_imgp_info_s->dst_height, dst);
		if(ret != MM_ERROR_NONE) {
			debug_error("__mm_util_imgp_finalize failed");
			IMGP_FREE(output_buffer);
			TTRACE_END();
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
		}
	} else {
		memcpy(dst, output_buffer, _imgp_info_s->buffer_size);
	}

	/* Output result*/
	debug_log("dst: %p dst_width: %d, dst_height: %d, output_stride: %d, output_elevation: %d",
			dst, _imgp_info_s->dst_width, _imgp_info_s->dst_height, _imgp_info_s->output_stride, _imgp_info_s->output_elevation);

	/* Finalize */
	ret = __mm_util_imgp_finalize(_module, _imgp_info_s);
	if(ret != MM_ERROR_NONE) {
		debug_error("__mm_util_imgp_finalize failed");
		IMGP_FREE(output_buffer);
		TTRACE_END();
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	IMGP_FREE(output_buffer);

	debug_fleave();

	TTRACE_END();

	return ret;
}

EXPORT_API int mm_util_resize_image(const unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format, unsigned char *dst, unsigned int *dst_width, unsigned int *dst_height)
{
	int ret = MM_ERROR_NONE;
	unsigned char *output_buffer = NULL;
	unsigned int output_buffer_size = 0;

	TTRACE_BEGIN("MM_UTILITY:IMGP:RESIZE_IMAGE");

	debug_fenter();

	if (!src || !dst) {
		debug_error("invalid argument");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if ((src_format < MM_UTIL_IMG_FMT_YUV420) || (src_format > MM_UTIL_IMG_FMT_NUM)) {
		debug_error("#ERROR# src_format value ");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if (!dst_width || !dst_height ) {
		debug_error("#ERROR# dst width/height buffer is NULL");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if (!src_width || !src_height) {
		debug_error("#ERROR# src_width || src_height valuei is 0 ");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	debug_log("[src] 0x%2x (%d x %d) [dst] 0x%2x", src, src_width, src_height, dst);

	imgp_info_s* _imgp_info_s=(imgp_info_s*)g_malloc0(sizeof(imgp_info_s));
	if(_imgp_info_s == NULL) {
		debug_error("ERROR - alloc handle");
		TTRACE_END();
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}

	IMGPInfoFunc _mm_util_imgp_func = NULL;
	GModule *_module = NULL;
	imgp_plugin_type_e _imgp_plugin_type_e = 0;

	/* Initialize */
	if(__mm_select_resize_plugin(src_format)) {
		_imgp_plugin_type_e = IMGP_NEON;
	}else {
		_imgp_plugin_type_e = IMGP_GSTCS;
	}
	debug_log("plugin type: %d", _imgp_plugin_type_e);

	_module = __mm_util_imgp_initialize(_imgp_plugin_type_e);
	debug_log("__mm_util_imgp_initialize: %p", _module);
	if(_module == NULL) /* when IMGP_NEON is NULL */
	{
		_imgp_plugin_type_e = IMGP_GSTCS;
		debug_log("You use %s module", PATH_GSTCS_LIB);
		_module = __mm_util_imgp_initialize(_imgp_plugin_type_e);
	}

	debug_log("__mm_set_imgp_info_s");
	ret =__mm_set_imgp_info_s(_imgp_info_s, src_format, src_width, src_height, src_format, *dst_width, *dst_height, MM_UTIL_ROTATE_0);
	if(ret != MM_ERROR_NONE) {
		debug_error("__mm_set_imgp_info_s failed [%d]", ret);
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	debug_log("Sucess __mm_set_imgp_info_s");

	if(g_strrstr(g_module_name (_module), GST)) {
		if(__mm_gst_can_resize_format(_imgp_info_s->input_format_label) == FALSE) {
			debug_error("[%s][%05d] #RESIZE ERROR# IMAGE_NOT_SUPPORT_FORMAT");
			__mm_util_imgp_finalize(_module, _imgp_info_s);
			TTRACE_END();
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
		}
	}

	ret = __mm_util_get_buffer_size(src_format, *dst_width, *dst_height, &output_buffer_size);
	if (ret != MM_ERROR_NONE) {
		debug_error("__mm_set_imgp_info_s failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return ret;
	}

	output_buffer = (unsigned char*)malloc(output_buffer_size);
	if (output_buffer == NULL) {
		debug_error("malloc failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}

	debug_log("malloc outputbuffer: %p (%d)", output_buffer, output_buffer_size);

	/* image processing */
	_mm_util_imgp_func = __mm_util_imgp_process(_module);
	debug_log("Sucess __mm_util_imgp_process");
	if (_mm_util_imgp_func) {
		ret=_mm_util_imgp_func(_imgp_info_s, src, output_buffer, IMGP_RSZ);
		debug_log("_mm_util_imgp_func, ret: %d", ret);
		if (ret != MM_ERROR_NONE)
		{
			debug_error("image processing failed");
			__mm_util_imgp_finalize(_module, _imgp_info_s);
			IMGP_FREE(output_buffer);
			TTRACE_END();
			return ret;
		}
	} else {
		debug_error("g_module_symbol failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		IMGP_FREE(output_buffer);
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if ((_imgp_info_s->dst_width != _imgp_info_s->output_stride || _imgp_info_s->dst_height != _imgp_info_s->output_elevation) && __mm_is_rgb_format(src_format)) {
		mm_util_crop_image(output_buffer, _imgp_info_s->output_stride, _imgp_info_s->output_elevation, src_format, 0, 0, &_imgp_info_s->dst_width, &_imgp_info_s->dst_height, dst);
		*dst_width = _imgp_info_s->dst_width;
		*dst_height = _imgp_info_s->dst_height;
	} else {
		memcpy(dst, output_buffer, _imgp_info_s->buffer_size);
		*dst_width = _imgp_info_s->dst_width;
		*dst_height = _imgp_info_s->dst_height;
	}

	/* Output result*/
	debug_log("dst: %p dst_width: %d, dst_height: %d, output_stride: %d, output_elevation: %d",
			dst, _imgp_info_s->dst_width, _imgp_info_s->dst_height, _imgp_info_s->output_stride, _imgp_info_s->output_elevation);

	/* Finalize */
	ret = __mm_util_imgp_finalize(_module, _imgp_info_s);
	if(ret != MM_ERROR_NONE) {
		debug_error("__mm_util_imgp_finalize failed");
		IMGP_FREE(output_buffer);
		TTRACE_END();
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	IMGP_FREE(output_buffer);

	debug_fenter();

	TTRACE_END();

	return ret;
}

EXPORT_API int mm_util_rotate_image(const unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format, unsigned char *dst, unsigned int *dst_width, unsigned int *dst_height, mm_util_img_rotate_type angle)
{
	int ret = MM_ERROR_NONE;
	unsigned char *output_buffer = NULL;
	unsigned int output_buffer_size = 0;

	TTRACE_BEGIN("MM_UTILITY:IMGP:ROTATE_IMAGE");

	debug_fenter();

	if (!src || !dst) {
		debug_error("invalid argument");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if ((src_format < MM_UTIL_IMG_FMT_YUV420) || (src_format > MM_UTIL_IMG_FMT_NUM)) {
		debug_error("#ERROR# src_format value");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if ( !dst_width || !dst_height ) {
		debug_error("#ERROR# dst width/height buffer is NUL");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if ( !src_width || !src_height) {
		debug_error("#ERROR# src_width || src_height value is 0 ");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if ((angle < MM_UTIL_ROTATE_0) || (angle > MM_UTIL_ROTATE_NUM)) {
		debug_error("#ERROR# angle vaule");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	debug_log("[src] 0x%2x (%d x %d) [dst] 0x%2x", src, src_width, src_height, dst);

	imgp_info_s* _imgp_info_s=(imgp_info_s*)g_malloc0(sizeof(imgp_info_s));
	if(_imgp_info_s == NULL) {
		debug_error("ERROR - alloc handle");
		TTRACE_END();
		return MM_ERROR_IMAGEHANDLE_NOT_INITIALIZED;
	}
	IMGPInfoFunc _mm_util_imgp_func = NULL;
	GModule *_module = NULL;
	imgp_plugin_type_e _imgp_plugin_type_e = 0;

	/* Initialize */
	if( __mm_select_rotate_plugin(src_format, src_width, src_height, angle)) {
		_imgp_plugin_type_e = IMGP_NEON;
	}else {
		_imgp_plugin_type_e = IMGP_GSTCS;
	}
	debug_log("plugin type: %d", _imgp_plugin_type_e);

	_module = __mm_util_imgp_initialize(_imgp_plugin_type_e);
	debug_log("__mm_util_imgp_initialize: %p", _module);
	if(_module == NULL) { /* when IMGP_NEON is NULL */
		_imgp_plugin_type_e = IMGP_GSTCS;
		debug_log("You use %s module", PATH_GSTCS_LIB);
		_module = __mm_util_imgp_initialize(_imgp_plugin_type_e);
	}

	debug_log("__mm_confirm_dst_width_height");
	ret = __mm_confirm_dst_width_height(src_width, src_height, dst_width, dst_height, angle);
	if(ret != MM_ERROR_NONE) {
		debug_error("dst_width || dest_height size Error");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	ret = __mm_set_imgp_info_s(_imgp_info_s, src_format, src_width, src_height, src_format, *dst_width, *dst_height, angle);
	debug_log("__mm_set_imgp_info_s");
	if(ret != MM_ERROR_NONE) {
		debug_error("__mm_set_imgp_info_s failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	debug_log("Sucess __mm_set_imgp_info_s");

	if(g_strrstr(g_module_name (_module), GST)) {
		if(__mm_gst_can_rotate_format(_imgp_info_s->input_format_label) == FALSE) {
			debug_error("[%s][%05d] #gstreamer ROTATE ERROR# IMAGE_NOT_SUPPORT_FORMAT");
			__mm_util_imgp_finalize(_module, _imgp_info_s);
			TTRACE_END();
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
		}
	}

	ret = __mm_util_get_buffer_size(src_format, *dst_width, *dst_height, &output_buffer_size);
	if (ret != MM_ERROR_NONE) {
		debug_error("__mm_set_imgp_info_s failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return ret;
	}

	output_buffer = (unsigned char*)malloc(output_buffer_size);
	if (output_buffer == NULL) {
		debug_error("malloc failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		TTRACE_END();
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}

	debug_log("malloc outputbuffer: %p (%d)", output_buffer, output_buffer_size);

	/* image processing */
	_mm_util_imgp_func = __mm_util_imgp_process(_module);
	debug_log("Sucess __mm_util_imgp_process");
	if (_mm_util_imgp_func) {
		ret=_mm_util_imgp_func(_imgp_info_s, src, output_buffer, IMGP_ROT);
		if (ret!= MM_ERROR_NONE) 	{
			debug_error("image processing failed");
			__mm_util_imgp_finalize(_module, _imgp_info_s);
			IMGP_FREE(output_buffer);
			TTRACE_END();
			return ret;
		}
	} else {
		debug_error("g_module_symbol failed");
		__mm_util_imgp_finalize(_module, _imgp_info_s);
		IMGP_FREE(output_buffer);
		TTRACE_END();
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	if ((_imgp_info_s->dst_width != _imgp_info_s->output_stride || _imgp_info_s->dst_height != _imgp_info_s->output_elevation) && __mm_is_rgb_format(src_format)) {
		unsigned int start_x = 0;
		unsigned int start_y = 0;
		if (angle == MM_UTIL_ROTATE_90) {
			start_x = 0;
			start_y = 0;
		} else if (angle == MM_UTIL_ROTATE_180) {
			start_x = _imgp_info_s->output_stride-_imgp_info_s->dst_width;
			start_y = 0;
		} else if (angle == MM_UTIL_ROTATE_270) {
			start_x = 0;
			start_y = _imgp_info_s->output_elevation - _imgp_info_s->dst_height;
		}
		mm_util_crop_image(output_buffer, _imgp_info_s->output_stride, _imgp_info_s->output_elevation, src_format, start_x, start_y, &_imgp_info_s->dst_width, &_imgp_info_s->dst_height, dst);
		*dst_width = _imgp_info_s->dst_width;
		*dst_height = _imgp_info_s->dst_height;
	} else {
		memcpy(dst, output_buffer, _imgp_info_s->buffer_size);
		*dst_width = _imgp_info_s->dst_width;
		*dst_height = _imgp_info_s->dst_height;
	}

	/* Output result*/
	debug_log("dst: %p dst_width: %d, dst_height: %d, output_stride: %d, output_elevation: %d",
			dst, _imgp_info_s->dst_width, _imgp_info_s->dst_height, _imgp_info_s->output_stride, _imgp_info_s->output_elevation);

	/* Finalize */
	ret = __mm_util_imgp_finalize(_module, _imgp_info_s);
	if(ret != MM_ERROR_NONE) {
		debug_error("__mm_util_imgp_finalize failed");
		IMGP_FREE(output_buffer);
		TTRACE_END();
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	IMGP_FREE(output_buffer);

	debug_fleave();

	TTRACE_END();

	return ret;
}

EXPORT_API int mm_util_crop_image(const unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int *crop_dest_width, unsigned int *crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;

	TTRACE_BEGIN("MM_UTILITY:IMGP:CROP_IMAGE");

	if (!src || !dst) {
		debug_error("invalid argument");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (src_format < MM_UTIL_IMG_FMT_YUV420) || (src_format > MM_UTIL_IMG_FMT_NUM) ) {
		debug_error("#ERROR# src_format value");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (crop_start_x +*crop_dest_width > src_width) || (crop_start_y +*crop_dest_height > src_height) ) {
		debug_error("#ERROR# dest width | height value");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	switch (src_format) {
		case MM_UTIL_IMG_FMT_RGB888: {
			ret = __mm_util_crop_rgb888(src, src_width, src_height, src_format, crop_start_x, crop_start_y, *crop_dest_width, *crop_dest_height, dst);
			break;
			}
		case MM_UTIL_IMG_FMT_RGB565: {
			ret = __mm_util_crop_rgb565(src, src_width, src_height, src_format, crop_start_x, crop_start_y, *crop_dest_width, *crop_dest_height, dst);
			break;
			}
		case MM_UTIL_IMG_FMT_ARGB8888:
		case MM_UTIL_IMG_FMT_BGRA8888:
		case MM_UTIL_IMG_FMT_RGBA8888:
		case MM_UTIL_IMG_FMT_BGRX8888: {
			ret = __mm_util_crop_rgba32(src, src_width, src_height, src_format, crop_start_x, crop_start_y, *crop_dest_width, *crop_dest_height, dst);
			break;
			}
		case MM_UTIL_IMG_FMT_I420:
		case MM_UTIL_IMG_FMT_YUV420: {
			if((*crop_dest_width %2) !=0) {
				debug_warning("#YUV Width value(%d) must be even at least# ", *crop_dest_width);
				*crop_dest_width = ((*crop_dest_width+1)>>1)<<1;
				debug_log("Image isplay is suceeded when YUV crop width value %d ",*crop_dest_width);
			}

			if((*crop_dest_height%2) !=0) { /* height value must be also even when crop yuv image */
				debug_warning("#YUV Height value(%d) must be even at least# ", *crop_dest_height);
				*crop_dest_height = ((*crop_dest_height+1)>>1)<<1;
				debug_log("Image isplay is suceeded when YUV crop height value %d ",*crop_dest_height);
			}

			ret = __mm_util_crop_yuv420(src, src_width, src_height, src_format, crop_start_x, crop_start_y, *crop_dest_width, *crop_dest_height, dst);
			break;
			}
		default:
			debug_log("Not supported format");
			ret = MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	TTRACE_END();
	return ret;
}

EXPORT_API int mm_util_get_image_size(mm_util_img_format format, unsigned int width, unsigned int height, unsigned int *imgsize)
{
	int ret = MM_ERROR_NONE;
	unsigned char x_chroma_shift = 0;
	unsigned char y_chroma_shift = 0;
	int size, w2, h2, size2;
	int stride, stride2;

	TTRACE_BEGIN("MM_UTILITY:IMGP:GET_SIZE");

	if (!imgsize) {
		debug_error("imgsize can't be null");
		TTRACE_END();
		return MM_ERROR_IMAGE_FILEOPEN;
	}

	*imgsize = 0;

	if (check_valid_picture_size(width, height) < 0) {
		debug_error("invalid width and height");
		TTRACE_END();
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	switch (format)
	{
		case MM_UTIL_IMG_FMT_I420:
		case MM_UTIL_IMG_FMT_YUV420:
			x_chroma_shift = 1;
			y_chroma_shift = 1;
			stride = MM_UTIL_ROUND_UP_4 (width);
			h2 = ROUND_UP_X (height, x_chroma_shift);
			size = stride * h2;
			w2 = DIV_ROUND_UP_X (width, x_chroma_shift);
			stride2 = MM_UTIL_ROUND_UP_4 (w2);
			h2 = DIV_ROUND_UP_X (height, y_chroma_shift);
			size2 = stride2 * h2;
			*imgsize = size + 2 * size2;
			break;
		case MM_UTIL_IMG_FMT_YUV422:
		case MM_UTIL_IMG_FMT_YUYV:
		case MM_UTIL_IMG_FMT_UYVY:
		case MM_UTIL_IMG_FMT_NV16:
		case MM_UTIL_IMG_FMT_NV61:
			stride = MM_UTIL_ROUND_UP_4 (width) * 2;
			size = stride * height;
			*imgsize = size;
			break;

		case MM_UTIL_IMG_FMT_RGB565:
			stride = MM_UTIL_ROUND_UP_4 (width) * 2;
			size = stride * height;
			*imgsize = size;
			break;

		case MM_UTIL_IMG_FMT_RGB888:
			stride = MM_UTIL_ROUND_UP_4 (width) * 3;
			size = stride * height;
			*imgsize = size;
			break;

		case MM_UTIL_IMG_FMT_ARGB8888:
		case MM_UTIL_IMG_FMT_BGRA8888:
		case MM_UTIL_IMG_FMT_RGBA8888:
		case MM_UTIL_IMG_FMT_BGRX8888:
			stride = width * 4;
			size = stride * height;
			*imgsize = size;
			break;


		case MM_UTIL_IMG_FMT_NV12:
		case MM_UTIL_IMG_FMT_NV12_TILED:
			x_chroma_shift = 1;
			y_chroma_shift = 1;
			stride = MM_UTIL_ROUND_UP_4 (width);
			h2 = ROUND_UP_X (height, y_chroma_shift);
			size = stride * h2;
			w2 = 2 * DIV_ROUND_UP_X (width, x_chroma_shift);
			stride2 = MM_UTIL_ROUND_UP_4 (w2);
			h2 = DIV_ROUND_UP_X (height, y_chroma_shift);
			size2 = stride2 * h2;
			*imgsize = size + size2;
			break;

		default:
			debug_error("Not supported format");
			TTRACE_END();
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	debug_log("format: %d, *imgsize: %d\n", format, *imgsize);

	TTRACE_END();
	return ret;
}


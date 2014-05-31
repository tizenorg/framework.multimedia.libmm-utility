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

#define MM_UTIL_ROUND_UP_2(num) (((num)+1)&~1)
#define MM_UTIL_ROUND_UP_4(num) (((num)+3)&~3)
#define MM_UTIL_ROUND_UP_8(num) (((num)+7)&~7)
#define MM_UTIL_ROUND_UP_16(num) (((num)+15)&~15)
#define GEN_MASK(x) ((1<<(x))-1)
#define ROUND_UP_X(v,x) (((v) + GEN_MASK(x)) & ~GEN_MASK(x))
#define DIV_ROUND_UP_X(v,x) (((v) + GEN_MASK(x)) >> (x))
#define GST "gstcs"

typedef gboolean(*IMGPInfoFunc) (imgp_info_s*, unsigned char*, unsigned char*, imgp_plugin_type_e);
/*########################################################################################*/

static int
check_valid_picture_size(int width, int height)
{
	if((int)width>0 && (int)height>0 && (width+128)*(unsigned long long)(height+128) < INT_MAX/4) {
		return MM_ERROR_NONE;
	}
	return MM_ERROR_IMAGE_INVALID_VALUE;
}

static gboolean
_mm_cannot_convert_format(mm_util_img_format src_format, mm_util_img_format dst_format )
{
	gboolean _bool=FALSE;
	debug_log("src_format: %d, dst_format:%d", src_format, dst_format);
	if(((src_format == MM_UTIL_IMG_FMT_YUV422) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_UYVY) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_RGB565) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_RGB888) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_BGRX8888) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_YUV422)) || ((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_UYVY)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_YUYV)) || ((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_BGRX8888)) ) {

		_bool = TRUE;
	}

	return _bool;
}

static gboolean
_mm_gst_can_resize_format(char* __format_label)
{
	gboolean _bool = FALSE;
	debug_log("Format label: %s",__format_label);
	if(strcmp(__format_label, "AYUV") == 0
		|| strcmp(__format_label, "UYVY") == 0 ||strcmp(__format_label, "Y800") == 0 || strcmp(__format_label, "I420") == 0 || strcmp(__format_label, "YV12") == 0
		|| strcmp(__format_label, "RGB888") == 0 || strcmp(__format_label, "RGB565") == 0 || strcmp(__format_label, "BGR888") == 0 || strcmp(__format_label, "RGBA8888") == 0
		|| strcmp(__format_label, "ARGB8888") == 0 ||strcmp(__format_label, "BGRA8888") == 0 ||strcmp(__format_label, "ABGR8888") == 0 ||strcmp(__format_label, "RGBX") == 0
		||strcmp(__format_label, "XRGB") == 0 ||strcmp(__format_label, "BGRX") == 0 ||strcmp(__format_label, "XBGR") == 0 ||strcmp(__format_label, "Y444") == 0
		||strcmp(__format_label, "Y42B") == 0 ||strcmp(__format_label, "YUY2") == 0 ||strcmp(__format_label, "YUYV") == 0 ||strcmp(__format_label, "UYVY") == 0
		||strcmp(__format_label, "Y41B") == 0 ||strcmp(__format_label, "Y16") == 0 ||strcmp(__format_label, "Y800") == 0 ||strcmp(__format_label, "Y8") == 0
		||strcmp(__format_label, "GREY") == 0 ||strcmp(__format_label, "AY64") == 0 || strcmp(__format_label, "YUV422") == 0) {

		_bool=TRUE;
	}
	return _bool;
}

static gboolean
_mm_gst_can_rotate_format(const char* __format_label)
{
	gboolean _bool = FALSE;
	debug_log("Format label: %s boolean: %d",__format_label, _bool);
	if(strcmp(__format_label, "I420") == 0 ||strcmp(__format_label, "YV12") == 0 || strcmp(__format_label, "IYUV") == 0
		|| strcmp(__format_label, "RGB888") == 0||strcmp(__format_label, "BGR888") == 0 ||strcmp(__format_label, "RGBA8888") == 0
		|| strcmp(__format_label, "ARGB8888") == 0 ||strcmp(__format_label, "BGRA8888") == 0 ||strcmp(__format_label, "ABGR8888") == 0 ) {
		_bool=TRUE;
	}
	debug_log("boolean: %d",_bool);
	return _bool;
}

static gboolean
_mm_select_convert_plugin(mm_util_img_format src_format, mm_util_img_format dst_format )
{
	gboolean _bool=FALSE;
	debug_log("src_format: %d, dst_format:%d", src_format, dst_format);
	if(((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_NV12)) || ((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||
		((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) || ((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) || ((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUV420) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_NV12)) || ((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||
		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) || ((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) ||
		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) || ((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_I420) && (dst_format == MM_UTIL_IMG_FMT_NV12_TILED)) ||

		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) || ((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) || ((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) || ((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||

		((src_format == MM_UTIL_IMG_FMT_UYVY) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) || ((src_format == MM_UTIL_IMG_FMT_UYVY) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||

		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) || ((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) || ((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_YUYV) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888)) ||

		((src_format == MM_UTIL_IMG_FMT_RGB565) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) || ((src_format == MM_UTIL_IMG_FMT_RGB565) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_RGB565) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||

		((src_format == MM_UTIL_IMG_FMT_RGB888) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) || ((src_format == MM_UTIL_IMG_FMT_RGB888) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_RGB888) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||

		((src_format == MM_UTIL_IMG_FMT_ARGB8888) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) || ((src_format == MM_UTIL_IMG_FMT_ARGB8888) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_ARGB8888) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||

		((src_format == MM_UTIL_IMG_FMT_BGRA8888) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) || ((src_format == MM_UTIL_IMG_FMT_BGRA8888) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_BGRA8888) && (dst_format == MM_UTIL_IMG_FMT_NV12)) ||

		((src_format == MM_UTIL_IMG_FMT_RGBA8888) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) || ((src_format == MM_UTIL_IMG_FMT_RGBA8888) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_RGBA8888) && (dst_format == MM_UTIL_IMG_FMT_NV12)) || ((src_format == MM_UTIL_IMG_FMT_RGBA8888) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||

		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_YUV420)) || ((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_I420)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_NV12)) || ((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_RGB565)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_RGB888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_ARGB8888)) || ((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_BGRA8888)) ||
		((src_format == MM_UTIL_IMG_FMT_NV12_TILED) && (dst_format == MM_UTIL_IMG_FMT_RGBA8888))) {

		_bool = TRUE;
	}

	return _bool;
}

static gboolean
_mm_select_resize_plugin(mm_util_img_format _format)
{
	gboolean _bool = FALSE;
	debug_log("_format: %d", _format);
	if( (_format == MM_UTIL_IMG_FMT_UYVY) || (_format == MM_UTIL_IMG_FMT_YUYV) || (_format == MM_UTIL_IMG_FMT_RGBA8888) || (_format == MM_UTIL_IMG_FMT_BGRX8888) ) {
		_bool = FALSE;
	}else {
		_bool = TRUE;
	}
	return _bool;
}

static gboolean
_mm_select_rotate_plugin(mm_util_img_format _format, unsigned int width, unsigned int height, mm_util_img_rotate_type angle)
{
	debug_log("_format: %d (angle: %d)", _format, angle);

	if((_format == MM_UTIL_IMG_FMT_YUV420) || (_format == MM_UTIL_IMG_FMT_I420) || (_format == MM_UTIL_IMG_FMT_NV12)
		||(( _format == MM_UTIL_IMG_FMT_RGB888 ||_format == MM_UTIL_IMG_FMT_RGB565) && angle == MM_UTIL_ROTATE_90)) { /* constraint of image processing because MM_UTIL_ROTATE_180 may be twice MM_UTIL_ROTATE_90 */
		return TRUE;
	}

	return FALSE;
}

static int
_mm_confirm_dst_width_height(unsigned int src_width, unsigned int src_height, unsigned int *dst_width, unsigned int *dst_height, mm_util_img_rotate_type angle)
{
	int ret = MM_ERROR_NONE;

	if(!dst_width || !dst_height) {
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
			debug_error("Not supported rotate value\n");
			return MM_ERROR_IMAGE_INVALID_VALUE;
	}
	return ret;
}

static int
_mm_set_format_label(imgp_info_s * _imgp_info_s, mm_util_img_format src_format, mm_util_img_format dst_format)
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
			src_fmt_lable = "YV12";
			break;
		case MM_UTIL_IMG_FMT_YUV422:
			src_fmt_lable = "Y42B";
			break;
		case MM_UTIL_IMG_FMT_I420:
			src_fmt_lable = "I420";
			break;
		case MM_UTIL_IMG_FMT_NV12:
			src_fmt_lable = "NV12";
			break;
		case MM_UTIL_IMG_FMT_UYVY:
			src_fmt_lable = "UYVY";
			break;
		case MM_UTIL_IMG_FMT_YUYV:
			src_fmt_lable = "YUYV";
			break;
		case MM_UTIL_IMG_FMT_RGB565:
			src_fmt_lable = "RGB565";
			break;
		case MM_UTIL_IMG_FMT_RGB888:
			src_fmt_lable = "RGB888";
			break;
		case MM_UTIL_IMG_FMT_ARGB8888:
			src_fmt_lable = "ARGB888";
			break;
		case MM_UTIL_IMG_FMT_BGRA8888:
			src_fmt_lable = "BGRA8888";
			break;
		case MM_UTIL_IMG_FMT_RGBA8888:
			src_fmt_lable = "RGBA8888";
			break;
		case MM_UTIL_IMG_FMT_BGRX8888:
			src_fmt_lable = "BGRX";
			break;
		default:
			debug_log("[%d] Not supported format", src_fmt_lable);
			break;
	}

	switch(dst_format) {
		case MM_UTIL_IMG_FMT_YUV420:
			dst_fmt_lable = "YV12";
			break;
		case MM_UTIL_IMG_FMT_YUV422:
			dst_fmt_lable = "Y42B";
			break;
		case MM_UTIL_IMG_FMT_I420:
			dst_fmt_lable = "I420";
			break;
		case MM_UTIL_IMG_FMT_NV12:
			dst_fmt_lable = "NV12";
			break;
		case MM_UTIL_IMG_FMT_UYVY:
			dst_fmt_lable = "UYVY";
			break;
		case MM_UTIL_IMG_FMT_YUYV:
			dst_fmt_lable = "YUYV";
			break;
		case MM_UTIL_IMG_FMT_RGB565:
			dst_fmt_lable = "RGB565";
			break;
		case MM_UTIL_IMG_FMT_RGB888:
			dst_fmt_lable = "RGB888";
			break;
		case MM_UTIL_IMG_FMT_ARGB8888:
			dst_fmt_lable = "ARGB8888";
			break;
		case MM_UTIL_IMG_FMT_BGRA8888:
			dst_fmt_lable = "BGRA8888";
			break;
		case MM_UTIL_IMG_FMT_RGBA8888:
			dst_fmt_lable = "RGBA8888";
			break;
		case MM_UTIL_IMG_FMT_BGRX8888:
			dst_fmt_lable = "BGRX";
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

static int
_mm_set_imgp_info_s(imgp_info_s * _imgp_info_s, mm_util_img_format src_format, unsigned int src_width, unsigned int src_height, mm_util_img_format dst_format, unsigned int dst_width, unsigned int dst_height, mm_util_img_rotate_type angle)
{
	int ret = MM_ERROR_NONE;
	if(_imgp_info_s == NULL) {
		debug_error("_imgp_info_s is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	ret=_mm_set_format_label(_imgp_info_s, src_format, dst_format);
	if(ret != MM_ERROR_NONE) {
		debug_error("[input] mm_set_format_label error");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	_imgp_info_s->src_format=src_format;
	_imgp_info_s->src_width = src_width;
	_imgp_info_s->src_height= src_height;

	_imgp_info_s->dst_format=dst_format;
	_imgp_info_s->dst_width = dst_width;
	_imgp_info_s->dst_height = dst_height;
	_imgp_info_s->angle= angle;

	debug_log("[input] format label: %s width: %d height: %d [output] format label: %s width: %d height: %d rotation_value: %d",
	_imgp_info_s->input_format_label, _imgp_info_s->src_width, _imgp_info_s->src_height,
	_imgp_info_s->output_format_label, _imgp_info_s->dst_width, _imgp_info_s->dst_height, _imgp_info_s->angle);

	return ret;
}

static GModule *
_mm_util_imgp_initialize(imgp_plugin_type_e _imgp_plugin_type_e)
{
	GModule *module = NULL;
	debug_log("#Start dlopen#");

	if( _imgp_plugin_type_e == IMGP_NEON ) {
		module = g_module_open(PATH_NEON_LIB, G_MODULE_BIND_LAZY);
	}else if( _imgp_plugin_type_e == IMGP_GSTCS) {
		module = g_module_open(PATH_GSTCS_LIB, G_MODULE_BIND_LAZY);
	}
	debug_log("#Success g_module_open#");
	if( module == NULL ) {
		debug_error("%s | %s module open failed", PATH_NEON_LIB, PATH_GSTCS_LIB);
		return NULL;
	}
	debug_log("module: %p, g_module_name: %s", module, g_module_name (module));
	return module;
}

static IMGPInfoFunc
_mm_util_imgp_process(GModule *module)
{
	IMGPInfoFunc mm_util_imgp_func = NULL;

	if(module == NULL) {
		debug_error("module is NULL");
		return NULL;
	}

	debug_log("#_mm_util_imgp_process#");

	g_module_symbol(module, IMGP_FUNC_NAME, (gpointer*)&mm_util_imgp_func);
	debug_log("mm_util_imgp_func: %p", mm_util_imgp_func);

	return mm_util_imgp_func;
}

static int
_mm_util_imgp_finalize(GModule *module, imgp_info_s *_imgp_info_s)
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

static int
_mm_util_crop_rgba32(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int crop_dest_width, unsigned int crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;
	int i;
	int src_bytesperline = src_width * 4;
	int dst_bytesperline = crop_dest_width * 4;

	debug_log("[Input] src: 0x%2x src, src_width: %d src_height: %d src_format: %d crop_start_x: %d crop_start_y: %d crop_dest_width: %d crop_dest_height: %d\n",
	src, src_width, src_height, src_format, crop_start_x, crop_start_y, crop_dest_width, crop_dest_height);
	src += crop_start_y * src_bytesperline + 4 * crop_start_x;

	for (i = 0; i < crop_dest_height; i++) {
		memcpy(dst, src, dst_bytesperline);
		src += src_bytesperline;
		dst += dst_bytesperline;
	}

	return ret;
}

static int
_mm_util_crop_rgb888(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int crop_dest_width, unsigned int crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;
	int i;
	int src_bytesperline = src_width * 3;
	int dst_bytesperline = crop_dest_width * 3;

	debug_log("[Input] src: 0x%2x src, src_width: %d src_height: %d src_format: %d crop_start_x: %d crop_start_y: %d crop_dest_width: %d crop_dest_height: %d\n",
	src, src_width, src_height, src_format, crop_start_x, crop_start_y, crop_dest_width, crop_dest_height);
	src += crop_start_y * src_bytesperline + 3 * crop_start_x;

	for (i = 0; i < crop_dest_height; i++) {
		memcpy(dst, src, dst_bytesperline);
		src += src_bytesperline;
		dst += dst_bytesperline;
	}

	return ret;
}

static int
_mm_util_crop_rgb565(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int crop_dest_width, unsigned int crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;
	int i;
	int src_bytesperline = src_width * 2;
	int dst_bytesperline = crop_dest_width * 2;

	debug_log("[Input] src: 0x%2x src, src_width: %d src_height: %d src_format: %d crop_start_x: %d crop_start_y: %d crop_dest_width: %d crop_dest_height: %d\n",
	src, src_width, src_height, src_format, crop_start_x, crop_start_y, crop_dest_width, crop_dest_height);
	src += crop_start_y * src_bytesperline + 2 * crop_start_x;

	for (i = 0; i < crop_dest_height; i++) {
		memcpy(dst, src, dst_bytesperline);
		src += src_bytesperline;
		dst += dst_bytesperline;
	}

	return ret;
}

static int
_mm_util_crop_yuv420(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int crop_dest_width, unsigned int crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;
	int i;
	int start_x = crop_start_x;
	int start_y = crop_start_y;
	debug_log("[Input] src: 0x%2x src, src_width: %d src_height: %d src_format: %d crop_start_x: %d crop_start_y: %d crop_dest_width: %d crop_dest_height: %d\n",
	src, src_width, src_height, src_format, crop_start_x, crop_start_y, crop_dest_width, crop_dest_height);
	unsigned char *_src = src + start_y * src_width + start_x;

	/* Y */
	for (i = 0; i < crop_dest_height; i++) {
		memcpy(dst, _src, crop_dest_width);
		_src += src_width;
		dst += crop_dest_width;
	}

	/* U */
	_src = src + src_height * src_width + (start_y / 2) * src_width / 2 + start_x / 2;
	for (i = 0; i < crop_dest_height / 2; i++) {
		memcpy(dst, _src, crop_dest_width / 2);
		_src += src_width / 2;
		dst += crop_dest_width / 2;
	}

	/* V */
	_src = src + src_height * src_width * 5 / 4 + (start_y / 2) * src_width / 2 + start_x / 2;
	for (i = 0; i < crop_dest_height / 2; i++) {
		memcpy(dst, _src, crop_dest_width / 2);
		_src += src_width / 2;
		dst += crop_dest_width / 2;
	}

	return ret;
}

EXPORT_API int
mm_util_convert_colorspace(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format, unsigned char *dst, mm_util_img_format dst_format)
{
	int ret = MM_ERROR_NONE;

	if(!src || !dst) {
		debug_error("invalid mm_util_convert_colorspace\n");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (src_format < MM_UTIL_IMG_FMT_YUV420) || (src_format > MM_UTIL_IMG_FMT_NUM) || (dst_format < MM_UTIL_IMG_FMT_YUV420) || (dst_format > MM_UTIL_IMG_FMT_NUM) ) {
		debug_error("#ERROR# src_format: %d || dst_format:%d value ", src_format, dst_format);
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	debug_log("#START#");

	if(_mm_cannot_convert_format(src_format, dst_format)) {
		debug_error("#ERROR# Cannot Support Image Format Convert");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	debug_log("[src] 0x%2x (%d x %d) [dst] 0x%2x", src, src_width, src_height, dst);

	imgp_info_s* _imgp_info_s=(imgp_info_s*)malloc(sizeof(imgp_info_s));
	IMGPInfoFunc _mm_util_imgp_func = NULL;
	GModule *_module = NULL;
	imgp_plugin_type_e _imgp_plugin_type_e=-1;

	/* Initialize */
	if(_mm_select_convert_plugin(src_format, dst_format)) {
		_imgp_plugin_type_e = IMGP_NEON;
	}else {
		_imgp_plugin_type_e = IMGP_GSTCS;
	}
	_module = _mm_util_imgp_initialize(_imgp_plugin_type_e);

	if(_module == NULL) { /* when IMGP_NEON is NULL */
		_imgp_plugin_type_e = IMGP_GSTCS;
		debug_log("You use %s module", PATH_GSTCS_LIB);
		_module = _mm_util_imgp_initialize(_imgp_plugin_type_e);
	}
	debug_log("mm_util_imgp_func: %p", _module);
	ret=_mm_set_imgp_info_s(_imgp_info_s, src_format, src_width, src_height, dst_format, src_width, src_height, MM_UTIL_ROTATE_0);
	if(ret != MM_ERROR_NONE) 	{
		debug_error("_mm_set_imgp_info_s failed");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}
	debug_log("Sucess _mm_set_imgp_info_s");

	/* image processing */
	_mm_util_imgp_func = _mm_util_imgp_process(_module);
	debug_log("Sucess _mm_util_imgp_process");

	if(_mm_util_imgp_func) {
		ret=_mm_util_imgp_func(_imgp_info_s, src, dst, IMGP_CSC);
		if (ret != MM_ERROR_NONE)
		{
			debug_error("image processing failed");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}
	}else {
		debug_error("g_module_symbol failed");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	/* Output result*/
	debug_log("dst: %p dst_width: %d, dst_height: %d, output_stride: %d, output_elevation: %d",
			dst, _imgp_info_s->dst_width, _imgp_info_s->dst_height, _imgp_info_s->output_stride, _imgp_info_s->output_elevation);
	debug_log("#Success# dst");

	/* Finalize */
	ret = _mm_util_imgp_finalize(_module, _imgp_info_s);
	if(ret != MM_ERROR_NONE) {
		debug_error("_mm_util_imgp_finalize failed");
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	return ret;
}

EXPORT_API int
mm_util_resize_image(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format, unsigned char *dst, unsigned int *dst_width, unsigned int *dst_height)
{
	int ret = MM_ERROR_NONE;
	if(!src || !dst) {
		debug_error("nvalid argument\n");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (src_format < MM_UTIL_IMG_FMT_YUV420) || (src_format > MM_UTIL_IMG_FMT_NUM) ) {
		debug_error("#ERROR# src_format value ");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( !dst_width || !dst_height ) {
		debug_error("#ERROR# dst width/height buffer is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (src_width < 0) || (src_height < 0)) {
		debug_error("#ERROR# src_width || src_height value ");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	debug_log("[src] 0x%2x (%d x %d) [dst] 0x%2x", src, src_width, src_height, dst);

	imgp_info_s* _imgp_info_s=(imgp_info_s*)malloc(sizeof(imgp_info_s));
	IMGPInfoFunc _mm_util_imgp_func = NULL;
	GModule *_module = NULL;
	imgp_plugin_type_e _imgp_plugin_type_e=-1;

	/* Initialize */
	if(_mm_select_resize_plugin(src_format)) {
		_imgp_plugin_type_e = IMGP_NEON;
	}else {
		_imgp_plugin_type_e = IMGP_GSTCS;
	}
	_module = _mm_util_imgp_initialize(_imgp_plugin_type_e);
	debug_log("_mm_util_imgp_init: %p", _module);
	if(_module == NULL) /* when IMGP_NEON is NULL */
	{
		_imgp_plugin_type_e = IMGP_GSTCS;
		debug_log("You use %s module", PATH_GSTCS_LIB);
		_module = _mm_util_imgp_initialize(_imgp_plugin_type_e);
	}
	debug_log("_mm_set_imgp_info_s");
	ret=_mm_set_imgp_info_s(_imgp_info_s, src_format, src_width, src_height, src_format, *dst_width, *dst_height, MM_UTIL_ROTATE_0);
	debug_log("_mm_set_imgp_info_s ret: %d", ret);
	if(ret != MM_ERROR_NONE) {
		debug_error("_mm_set_imgp_info_s failed");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}
	debug_log("Sucess _mm_set_imgp_info_s");

	if(g_strrstr(g_module_name (_module), GST)) {
		if(_mm_gst_can_resize_format(_imgp_info_s->input_format_label) == FALSE) {
			debug_error("[%s][%05d] #RESIZE ERROR# IMAGE_NOT_SUPPORT_FORMAT");
			_mm_util_imgp_finalize(_module, _imgp_info_s);
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
		}
	}

	/* image processing */
	_mm_util_imgp_func = _mm_util_imgp_process(_module);
	debug_log("Sucess _mm_util_imgp_process");
	if(_mm_util_imgp_func) {
		ret=_mm_util_imgp_func(_imgp_info_s, src, dst, IMGP_RSZ);
		debug_log("_mm_util_imgp_func, ret: %d", ret);
		if (ret != MM_ERROR_NONE)
		{
			debug_error("image processing failed");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}
	}else {
		debug_error("g_module_symbol failed");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	/* Output result*/
	debug_log("dst: %p dst_width: %d, dst_height: %d, output_stride: %d, output_elevation: %d",
			dst, _imgp_info_s->dst_width, _imgp_info_s->dst_height, _imgp_info_s->output_stride, _imgp_info_s->output_elevation);
	debug_log("#Success# dst");

	*dst_width = _imgp_info_s->dst_width;
	*dst_height = _imgp_info_s->dst_height;

	/* Finalize */
	ret = _mm_util_imgp_finalize(_module, _imgp_info_s);
	if(ret != MM_ERROR_NONE) 	{
		debug_error("_mm_util_imgp_finalize failed");
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	return ret;
}

EXPORT_API int
mm_util_rotate_image(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format, unsigned char *dst, unsigned int *dst_width, unsigned int *dst_height, mm_util_img_rotate_type angle)
{
	int ret = MM_ERROR_NONE;

	if(!src || !dst) {
		debug_error("invalid argument\n");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (src_format < MM_UTIL_IMG_FMT_YUV420) || (src_format > MM_UTIL_IMG_FMT_NUM) ) {
		debug_error("#ERROR# src_format value");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( !dst_width || !dst_height ) {
		debug_error("#ERROR# dst width/height buffer is NUL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (src_width < 0) || (src_height < 0)) {
		debug_error("#ERROR# src_width || src_height value ");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (angle < MM_UTIL_ROTATE_0) || (angle > MM_UTIL_ROTATE_NUM) ) {
		debug_error("#ERROR# angle vaule");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	debug_log("[src] 0x%2x (%d x %d) [dst] 0x%2x", src, src_width, src_height, dst);

	debug_log("#START#");
	imgp_info_s* _imgp_info_s=(imgp_info_s*)malloc(sizeof(imgp_info_s));
	unsigned int dst_size=0;
	IMGPInfoFunc _mm_util_imgp_func = NULL;
	GModule *_module = NULL;
	imgp_plugin_type_e _imgp_plugin_type_e=-1;

	/* Initialize */
	if( _mm_select_rotate_plugin(src_format, src_width, src_height, angle)) {
		_imgp_plugin_type_e = IMGP_NEON;
	}else {
		_imgp_plugin_type_e = IMGP_GSTCS;
	}
	_module = _mm_util_imgp_initialize(_imgp_plugin_type_e);
	debug_log("_mm_util_imgp_func: %p", _module);
	if(_module == NULL) { /* when IMGP_NEON is NULL */
		_imgp_plugin_type_e = IMGP_GSTCS;
		debug_log("You use %s module", PATH_GSTCS_LIB);
		_module = _mm_util_imgp_initialize(_imgp_plugin_type_e);
	}
	debug_log("_mm_set_imgp_info_s");
	ret=_mm_confirm_dst_width_height(src_width, src_height, dst_width, dst_height, angle);
	if(ret != MM_ERROR_NONE) {
		debug_error("dst_width || dest_height size Error");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	ret=_mm_set_imgp_info_s(_imgp_info_s, src_format, src_width, src_height, src_format, *dst_width, *dst_height, angle);
	debug_log("_mm_set_imgp_info_s");
	if(ret != MM_ERROR_NONE) {
		debug_error("_mm_set_imgp_info_s failed");
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	debug_log("Sucess _mm_set_imgp_info_s");

	if(g_strrstr(g_module_name (_module), GST)) {
		if(_mm_gst_can_rotate_format(_imgp_info_s->input_format_label) == FALSE) {
			debug_error("[%s][%05d] #gstreamer ROTATE ERROR# IMAGE_NOT_SUPPORT_FORMAT");
			_mm_util_imgp_finalize(_module, _imgp_info_s);
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
		}
	}

	/* image processing */
	_mm_util_imgp_func = _mm_util_imgp_process(_module);
	debug_log("Sucess _mm_util_imgp_process");
	if(_mm_util_imgp_func) {
		ret=_mm_util_imgp_func(_imgp_info_s, src, dst, IMGP_ROT);
		if (ret!= MM_ERROR_NONE) 	{
			debug_error("image processing failed");
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
		}
	}else {
		debug_error("g_module_symbol failed");
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	/* Output result*/
	debug_log("dst: %p dst_width: %d, dst_height: %d, output_stride: %d, output_elevation: %d, dst_size: %d",
			dst, _imgp_info_s->dst_width, _imgp_info_s->dst_height, _imgp_info_s->output_stride, _imgp_info_s->output_elevation,dst_size);
	debug_log("#Success# dst");

	*dst_width = _imgp_info_s->dst_width;
	*dst_height = _imgp_info_s->dst_height;

	/* Finalize */
	ret = _mm_util_imgp_finalize(_module, _imgp_info_s);
	if(ret != MM_ERROR_NONE) {
		debug_error("_mm_util_imgp_finalize failed");
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	return ret;
}

EXPORT_API int
mm_util_crop_image(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
unsigned int crop_start_x, unsigned int crop_start_y, unsigned int *crop_dest_width, unsigned int *crop_dest_height, unsigned char *dst)
{
	int ret = MM_ERROR_NONE;

	if (!src || !dst) {
		debug_error("invalid argument\n");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (src_format < MM_UTIL_IMG_FMT_YUV420) || (src_format > MM_UTIL_IMG_FMT_NUM) ) {
		debug_error("#ERROR# src_format value");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (crop_start_x +*crop_dest_width > src_width) || (crop_start_y +*crop_dest_height > src_height) ) {
		debug_error("#ERROR# dest width | height value");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	switch (src_format) {
		case MM_UTIL_IMG_FMT_RGB888: {
			ret = _mm_util_crop_rgb888(src, src_width, src_height, src_format, crop_start_x, crop_start_y, *crop_dest_width, *crop_dest_height, dst);
			break;
			}
		case MM_UTIL_IMG_FMT_RGB565: {
			ret = _mm_util_crop_rgb565(src, src_width, src_height, src_format, crop_start_x, crop_start_y, *crop_dest_width, *crop_dest_height, dst);
			break;
			}
		case MM_UTIL_IMG_FMT_ARGB8888:
		case MM_UTIL_IMG_FMT_BGRA8888:
		case MM_UTIL_IMG_FMT_RGBA8888:
		case MM_UTIL_IMG_FMT_BGRX8888: {
			ret = _mm_util_crop_rgba32(src, src_width, src_height, src_format, crop_start_x, crop_start_y, *crop_dest_width, *crop_dest_height, dst);
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

			ret = _mm_util_crop_yuv420(src, src_width, src_height, src_format, crop_start_x, crop_start_y, *crop_dest_width, *crop_dest_height, dst);
			break;
			}
		default:
			debug_log("Not supported format");
	}

	return ret;
}

EXPORT_API int
mm_util_get_image_size(mm_util_img_format format, unsigned int width, unsigned int height, unsigned int *imgsize)
{
	int ret = MM_ERROR_NONE;
	unsigned char x_chroma_shift = 0;
	unsigned char y_chroma_shift = 0;
	int size, w2, h2, size2;
	int stride, stride2;

	if (!imgsize) {
		debug_error("imgsize can't be null\n");
		return MM_ERROR_IMAGE_FILEOPEN;
	}

	*imgsize = 0;


	if (check_valid_picture_size(width, height) < 0) {
		debug_error("invalid width and height\n");
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
			stride = MM_UTIL_ROUND_UP_4 (width * 2);
			size = stride * height;
			*imgsize = size;
			break;

		case MM_UTIL_IMG_FMT_RGB565:
			stride = MM_UTIL_ROUND_UP_4 (width * 2);
			size = stride * height;
			*imgsize = size;
			break;

		case MM_UTIL_IMG_FMT_RGB888:
			stride = MM_UTIL_ROUND_UP_4 (width * 3);
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
			debug_error("Not supported format\n");
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	debug_log("format: %d, *imgsize: %d\n", format, *imgsize);

	return ret;
}

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include "mm_util_imgp.h"
#ifdef __cplusplus
	extern "C" {
#endif

#include <gmodule.h>
#include <mm_debug.h>
#include <mm_types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
/* tbm */
#include <tbm_surface_internal.h>

#define PATH_NEON_LIB					"/usr/lib/libmmutil_imgp_neon.so"
#define PATH_GSTCS_LIB					"/usr/lib/libmmutil_imgp_gstcs.so"

#define IMGP_FUNC_NAME  				"mm_imgp"
#define IMGP_FREE(src) { if(src) {g_free(src); src = NULL;} }
#define SCMN_IMGB_MAX_PLANE         (4)
#define MAX_SRC_BUF_NUM          12	/* Max number of upstream src plugins's buffer */
#define MAX_DST_BUF_NUM          12
#define SCMN_Y_PLANE                 0
#define SCMN_CB_PLANE                1
#define SCMN_CR_PLANE                2
#define MM_UTIL_DST_WIDTH_DEFAULT 0
#define MM_UTIL_DST_WIDTH_MIN 0
#define MM_UTIL_DST_WIDTH_MAX 32767
#define MM_UTIL_DST_HEIGHT_DEFAULT 0
#define MM_UTIL_DST_HEIGHT_MIN 0
#define MM_UTIL_DST_HEIGHT_MAX 32767
#define MM_UTIL_DST_BUFFER_MIN 2
#define MM_UTIL_DST_BUFFER_DEFAULT 3
#define MM_UTIL_DST_BUFFER_MAX 12
#define MAX_IPP_EVENT_BUFFER_SIZE 1024

/* macro for align ***********************************************************/
#define ALIGN_TO_2B(x)		((((x)  + (1 <<  1) - 1) >>  1) <<  1)
#define ALIGN_TO_4B(x)		((((x)  + (1 <<  2) - 1) >>  2) <<  2)
#define ALIGN_TO_8B(x)		((((x)  + (1 <<  3) - 1) >>  3) <<  3)
#define ALIGN_TO_16B(x)		((((x)  + (1 <<  4) - 1) >>  4) <<  4)
#define ALIGN_TO_32B(x)		((((x)  + (1 <<  5) - 1) >>  5) <<  5)
#define ALIGN_TO_128B(x)	((((x)  + (1 <<  7) - 1) >>  7) <<  7)
#define ALIGN_TO_2KB(x)		((((x)  + (1 << 11) - 1) >> 11) << 11)
#define ALIGN_TO_8KB(x)		((((x)  + (1 << 13) - 1) >> 13) << 13)
#define ALIGN_TO_64KB(x)	((((x)  + (1 << 16) - 1) >> 16) << 16)

/**
 * Image Process Info for dlopen
 */
typedef struct _imgp_info_s
{
	char *input_format_label;
	mm_util_img_format src_format;
	unsigned int src_width;
	unsigned int src_height;
	char *output_format_label;
	mm_util_img_format dst_format;
	unsigned int dst_width;
	unsigned int dst_height;
	unsigned int output_stride;
	unsigned int output_elevation;
	mm_util_img_rotate_type angle;
} imgp_info_s;

/* Enumerations */
typedef enum
{
	IMGP_CSC = 0,
	IMGP_RSZ,
	IMGP_ROT,
} imgp_type_e;

/* Enumerations */
typedef enum
{
	IMGP_NEON = 0,
	IMGP_GSTCS,
} imgp_plugin_type_e;

typedef enum
{
    MM_UTIL_ROTATION_NONE = 0,  /**< None */
    MM_UTIL_ROTATION_90 = 1,    /**< Rotation 90 degree */
    MM_UTIL_ROTATION_180,       /**< Rotation 180 degree */
    MM_UTIL_ROTATION_270,       /**< Rotation 270 degree */
    MM_UTIL_ROTATION_FLIP_HORZ, /**< Flip horizontal */
    MM_UTIL_ROTATION_FLIP_VERT, /**< Flip vertical */
} mm_util_rotation_e;

typedef struct
{
	void *user_data;
	mm_util_completed_callback completed_cb;
} mm_util_cb_s;

typedef enum {
	GEM_FOR_SRC = 0,
	GEM_FOR_DST = 1,
} ConvertGemCreateType;

typedef enum {
	MM_UTIL_IPP_CTRL_STOP = 0,
	MM_UTIL_IPP_CTRL_START = 1,
} ConvertIppCtrl;

typedef struct
{
	gint drm_fd;
	media_packet_h src_packet;
	void *src;
	mm_util_img_format src_format;
	unsigned int src_width;
	unsigned int src_height;
	tbm_surface_h surface;
	media_packet_h dst_packet;
	void *dst;
	mm_util_img_format dst_format;
	media_format_mimetype_e dst_format_mime;
	unsigned int start_x;
	unsigned int start_y;
	unsigned int dst_width;
	unsigned int dst_height;
	mm_util_rotation_e dst_rotation;

	bool hardware_acceleration;
	mm_util_cb_s *_util_cb;
	bool is_completed;

	tbm_bufmgr tbm;
	tbm_bo src_bo;
	unsigned int src_key;
	tbm_bo_handle src_bo_handle;
	tbm_bo dst_bo;
	unsigned int dst_key;
	tbm_bo_handle dst_bo_handle;

	/* Src paramters */
	guint src_buf_size; /**< for a standard colorspace format */
	/* Dst paramters */
	guint dst_buf_size;

	/* Properties */

	/* DRM/GEM information */
	guint src_buf_idx;
	guint dst_buf_idx;

	/* for multi instance */
	GMutex * instance_lock;
	GMutex *fd_lock;
	GMutex *buf_idx_lock;
} mm_util_s;

#ifdef __cplusplus
}
#endif

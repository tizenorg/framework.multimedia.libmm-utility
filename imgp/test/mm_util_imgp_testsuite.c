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
#include "mm_util_imgp.h"
#include "mm_util_imgp_internal.h"
#include <mm_ta.h>
#include <mm_error.h>
#define ONE_ALL 0
#define IMAGE_FORMAT_LABEL_BUFFER_SIZE 4

int main(int argc, char *argv[])
{
	unsigned char *src = NULL;
	unsigned char *dst = NULL;
	unsigned int src_width = 0;
	unsigned int src_height = 0;
	unsigned int dst_width = 0;
	unsigned int dst_height = 0;
	char output_file[25];
	unsigned int src_size = 0;
	unsigned int dst_size = 0;
	int src_cs = 0;
	int dst_cs = 0;
	int ret = 0;
	int cnt = 0;
	char *fmt = NULL;
	FILE *fp = NULL;

	if (argc < 6) {
		debug_error("[%s][%05d] Usage: ./mm_image_testsuite filename [yuv420 | yuv420p | yuv422 | uyvy | vyuy | nv12 | nv12t | rgb565 | rgb888 | argb | jpeg] width height\n");
		exit (0);
	}

	MMTA_INIT();
	#if ONE_ALL
	while(cnt++ < 10000) {
	#endif

	if (!strcmp("convert", argv[1])) {
		FILE *fp = fopen(argv[2], "r");
		src_width = atoi(argv[3]);
		src_height = atoi(argv[4]);
		dst_width = atoi(argv[5]);
		dst_height = atoi(argv[6]);
		src_cs =atoi(argv[7]);
		dst_cs = atoi(argv[8]);
		debug_log("convert [FILE (%s)] [src_width (%d)] [src_height (%d)] [dst_width (%d)] [dst_height (%d)] [src_cs (%d)] [dst_cs (%d)]",
		argv[2], src_width, src_height, dst_width, dst_height, src_cs, dst_cs);

		ret = mm_util_get_image_size(src_cs, src_width, src_height, &src_size);
		debug_log("convert src buffer size=%d\n", src_size);
		src = malloc(src_size);
		if(fread(src, 1, src_size, fp)) {
			debug_log("#Success# fread");
		} else {
			debug_error("#Error# fread");
		}

		ret = mm_util_get_image_size(dst_cs, dst_width, dst_height, &dst_size);
		debug_log("dst_cs: %d dst_width: %d dst_height: %d dst buffer size=%d\n", dst_cs, dst_width, dst_height, dst_size);
		dst = malloc(dst_size);
		debug_log("dst: %p", dst);

		__ta__("mm_util_convert_colorspace",
		ret = mm_util_convert_colorspace(src, src_width, src_height, src_cs, dst, dst_cs);
		);
		debug_log("mm_util_convert_colorspace dst:%p ret = %d", dst, ret);
	} else if (!strcmp("resize", argv[1])) {
		fp = fopen(argv[2], "r");

		src_width = atoi(argv[3]);
		src_height = atoi(argv[4]);
		dst_width = atoi(argv[5]);
		dst_height = atoi(argv[6]);
		src_cs =atoi(argv[7]);
		dst_cs = src_cs;
		debug_log("resize [FILE (%s)] [src_width (%d)] [src_height (%d)] [dst_width (%d)] [dst_height (%d)] [src_cs (%d)]",
		argv[2], src_width, src_height, dst_width, dst_height, src_cs);

		ret = mm_util_get_image_size(src_cs, src_width, src_height, &src_size);
		debug_log("convert src buffer size=%d\n", src_size);
		src = malloc(src_size);
		if(fread(src, 1, src_size, fp)) {
			debug_log("#Success# fread");
		} else {
			debug_error("#Error# fread");
		}

		ret = mm_util_get_image_size(dst_cs, dst_width, dst_height, &dst_size);
		debug_log("dst_cs: %d dst_width: %d dst_height: %d dst buffer size=%d\n", dst_cs, dst_width, dst_height, dst_size);
		dst = malloc(dst_size);
		debug_log("dst: %p", dst);

		__ta__("mm_util_resize_image",
		ret = mm_util_resize_image(src, src_width, src_height, src_cs, dst, &dst_width, &dst_height);
		);
		debug_log("mm_util_resize_image dst: %p ret = %d", dst, ret);
	} else if (!strcmp("rotate", argv[1])) {
		fp = fopen(argv[2], "r");
		src_width = atoi(argv[3]);
		src_height = atoi(argv[4]);
		dst_width = atoi(argv[5]);
		dst_height = atoi(argv[6]);
		src_cs =atoi(argv[7]);
		mm_util_img_rotate_type angle = atoi(argv[8]);
		dst_cs = src_cs;
		debug_log("rotate [FILE (%s)] [src_width (%d)] [src_height (%d)] [dst_width (%d)] [dst_height (%d)] [src_cs (%d)] [angle (%d)]",
		argv[2], src_width, src_height, dst_width, dst_height, src_cs, angle);

		ret = mm_util_get_image_size(src_cs, src_width, src_height, &src_size);
		debug_log("convert src buffer size=%d\n", src_size);
		src = malloc(src_size);
		if(fread(src, 1, src_size, fp)) {
			debug_log("#Success# fread");
		} else {
			debug_error("#Error# fread");
		}

		ret = mm_util_get_image_size(dst_cs, dst_width, dst_height, &dst_size);
		debug_log("dst_cs: %d dst_width: %d dst_height: %d dst buffer size=%d\n", dst_cs, dst_width, dst_height, dst_size);
		dst = malloc(dst_size);
		debug_log("dst: %p", dst);

		__ta__("mm_util_rotate_image",
		ret = mm_util_rotate_image(src, src_width, src_height, src_cs, dst, &dst_width, &dst_height, angle);
		);
		debug_log("mm_util_rotate_image dst: %p ret = %d\n", dst, ret);
	} else if (!strcmp("crop", argv[1])) {
		fp = fopen(argv[2], "r");
		src_width = atoi(argv[3]);
		src_height = atoi(argv[4]);
		unsigned int crop_start_x = atoi(argv[5]);
		unsigned int crop_start_y = atoi(argv[6]);
		dst_width = atoi(argv[7]);
		dst_height = atoi(argv[8]);
		src_cs =atoi(argv[9]);
		dst_cs = src_cs;
		debug_log("rotate [FILE (%s)] [src_width (%d)] [src_height (%d)] [crop_start_x (%d)] [crop_start_y (%d)] [dst_width (%d)] [dst_height (%d)] [src_cs (%d)]", argv[2], src_width, src_height, crop_start_x, crop_start_y, dst_width, dst_height, src_cs);

		ret = mm_util_get_image_size(src_cs, src_width, src_height, &src_size);
		debug_log("convert src buffer size=%d\n", src_size);
		src = malloc(src_size);
		if(fread(src, 1, src_size, fp)) {
			debug_log("#Success# fread");
		} else {
			debug_error("#Error# fread");
		}

		ret = mm_util_get_image_size(dst_cs, dst_width, dst_height, &dst_size);
		debug_log("dst_cs: %d dst_width: %d dst_height: %d dst buffer size=%d\n", dst_cs, dst_width, dst_height, dst_size);
		dst = malloc(dst_size);
		debug_log("dst: %p", dst);

		__ta__("mm_util_crop_image",
		ret = mm_util_crop_image(src, src_width, src_height, src_cs, crop_start_x, crop_start_y, &dst_width, &dst_height, dst);
		);
		debug_log("mm_util_crop_image dst: %p ret = %d\n", dst, ret);
	} else {
		debug_error("convert | resize | rotate | crop fail\n");
	}

	MMTA_ACUM_ITEM_SHOW_RESULT();
	MMTA_RELEASE ();

	FILE *fpout = NULL;

	if(ret==MM_ERROR_NONE) {
		if(cnt == 0) {
			fmt = (char*)malloc(sizeof(char) * IMAGE_FORMAT_LABEL_BUFFER_SIZE);
			if(fmt) {
				memset(fmt, 0, IMAGE_FORMAT_LABEL_BUFFER_SIZE);
				if(dst_cs ==MM_UTIL_IMG_FMT_YUV420 || dst_cs == MM_UTIL_IMG_FMT_YUV422 ||dst_cs == MM_UTIL_IMG_FMT_I420
					|| dst_cs == MM_UTIL_IMG_FMT_NV12 || dst_cs == MM_UTIL_IMG_FMT_UYVY ||dst_cs == MM_UTIL_IMG_FMT_YUYV) {
					strncpy(fmt, "yuv", strlen("yuv"));
				} else {
					strncpy(fmt,"rgb", strlen("rgb"));
				}
				sprintf(output_file, "result%d_%dx%d.%s", cnt, dst_width, dst_height, fmt);
				fpout = fopen(output_file, "w");
				debug_log("%s = %dx%d dst_size: %d", output_file, dst_width, dst_height, dst_size);
				debug_log("dst:%p ret = %d", dst, ret);
				fwrite(dst, 1, dst_size, fpout);
				fflush(fpout);
				free(fmt);
				fmt=NULL;
			}
		}
		if(fp) {
		fclose(fp);
		debug_log("fclose(fp) fp: 0x%2x", fp);
	}

	if(fpout) {
		fclose(fpout);
		debug_log("fclose(fp) fpout: 0x%2x", fpout);
	}

	debug_log("src: %p", src);
	if(src) {
		free(src); src = NULL;
	}
	debug_log("dst: %p",dst);
	if(dst) {
		free(dst); dst = NULL;
	}
	debug_log("Success -  free src & dst");

	#if ONE_ALL
	debug_log("cnt: %d", cnt);
	}
	#endif
	}

	return ret;
}


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
#include <mm_error.h>
#define ONE_ALL 0
#define IMAGE_FORMAT_LABEL_BUFFER_SIZE 4
MMHandleType MMHandle = 0;

bool
transform_completed_cb(media_packet_h *packet, int error, void *user_param)
{
	uint64_t size = 0;
	char output_file[25] = {};
	debug_log("MMHandle: 0x%2x", MMHandle);

	media_format_h dst_fmt;
	media_format_mimetype_e dst_mimetype;
	int dst_width, dst_height, dst_avg_bps, dst_max_bps;
	char *output_fmt = (char*)malloc(sizeof(char) * IMAGE_FORMAT_LABEL_BUFFER_SIZE);
	if(output_fmt) {
		if(media_packet_get_format(*packet, &dst_fmt) != MM_ERROR_NONE) {
			debug_error("Imedia_packet_get_format");
			IMGP_FREE(output_fmt);
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_get_video_info(dst_fmt, &dst_mimetype, &dst_width, &dst_height, &dst_avg_bps, &dst_max_bps) ==MEDIA_FORMAT_ERROR_NONE) {
			memset(output_fmt, 0, IMAGE_FORMAT_LABEL_BUFFER_SIZE);
			if(dst_mimetype  ==MEDIA_FORMAT_YV12 || dst_mimetype == MEDIA_FORMAT_422P ||dst_mimetype == MEDIA_FORMAT_I420
				|| dst_mimetype == MEDIA_FORMAT_NV12 || dst_mimetype == MEDIA_FORMAT_UYVY ||dst_mimetype == MEDIA_FORMAT_YUYV) {
				strncpy(output_fmt, "yuv", strlen("yuv"));
			} else {
				strncpy(output_fmt,"rgb", strlen("rgb"));
			}
			debug_log("[mimetype: %d] W x H : %d x %d", dst_mimetype, dst_width, dst_height);
			sprintf(output_file, "result_%dx%d.%s", dst_width,dst_height, output_fmt);
		}
	}

	if(error == MM_ERROR_NONE) {
		debug_log("completed");
	} else {
		debug_error("[ERROR] complete cb");
		GThread * destroy_thread = g_thread_new(NULL, mm_util_destroy, MMHandle);
		g_thread_unref(destroy_thread);
	}

	FILE *fpout = fopen(output_file, "w");
	if(fpout) {
		media_packet_get_buffer_size(*packet, &size);
		void *dst = NULL;
		if(media_packet_get_buffer_data_ptr(*packet, &dst) != MM_ERROR_NONE) {
			IMGP_FREE(dst);
			IMGP_FREE(output_fmt);
			fclose(fpout);
			debug_error ("[dst] media_packet_get_extra");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}
		debug_log("dst: %p [%d]", dst, size);
		fwrite(dst, 1, size, fpout);
		debug_log("FREE");
		fclose(fpout);
	}

	debug_log("write result");
	debug_log("Free (output_fmt)");
	media_packet_destroy(*packet);
	IMGP_FREE(output_fmt);
	return TRUE;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	mm_util_s *handle = NULL;
	void *src;
	void *ptr;
	media_packet_h src_packet;

	if (argc < 1) {
		debug_error("[%s][%05d] Usage: ./mm_image_testsuite filename [yuv420 | yuv420p | yuv422 | uyvy | vyuy | nv12 | nv12t | rgb565 | rgb888 | argb | jpeg] width height\n");
		exit (0);
	}


	/* Create Transform */
	ret = mm_util_create (&MMHandle);
	if(ret == MM_ERROR_NONE) {
		debug_log("Success - Create Transcode Handle [MMHandle: 0x%2x]", MMHandle);
	} else {
		debug_log("ERROR - Create Transcode Handle");
		return ret;
	}

	handle = (mm_util_s*) MMHandle;

	media_format_h fmt;
	if(media_format_create(&fmt) == MEDIA_FORMAT_ERROR_NONE) {
		if(media_format_set_video_mime(fmt, MEDIA_FORMAT_I420) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(fmt);
			debug_error("[Error] Set - video mime");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_set_video_width(fmt, 320) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(fmt);
			debug_error("[Error] Set - video width");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_set_video_height(fmt, 240) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(fmt);
			debug_error("[Error] Set - video height");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_set_video_avg_bps(fmt, 15000000) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(fmt);
			debug_error("[Error] Set - video avg bps");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		if(media_format_set_video_max_bps(fmt, 20000000) != MEDIA_FORMAT_ERROR_NONE) {
			media_format_unref(fmt);
			debug_error("[Error] Set - video max bps");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}

		debug_log("media_format_set_video_info success! w:320, h:240, MEDIA_FORMAT_I420\n");
	}
	else {
		debug_error("media_format_create failed...");
	}

	ret = media_packet_create_alloc(fmt, (media_packet_finalize_cb)transform_completed_cb, NULL, &src_packet);
	if(ret == MM_ERROR_NONE) {
		debug_log("Success - Create Media Packet(%p)", src_packet);
		uint64_t size =0;
		if (media_packet_get_buffer_size(src_packet, &size) == MEDIA_PACKET_ERROR_NONE) {
			ptr = malloc(size);
			if (media_packet_get_buffer_data_ptr(src_packet, &ptr) == MEDIA_PACKET_ERROR_NONE) {
				FILE *fp = fopen(argv[1], "r");
				src = malloc(size);
				if(fread(src, 1, (int)size, fp)) {
					debug_log("#Success# fread");
					memcpy(ptr, src, (int)size);
					debug_log("memcpy");
				} else {
					debug_error("#Error# fread");
				}
			}
		}
	} else {
		debug_log("ERROR - Create Media Packet");
		return ret;
	}

	/* Set Source */
	ret = mm_util_set_hardware_acceleration(MMHandle, atoi(argv[2]));
	if(ret == MM_ERROR_NONE) {
		debug_log("Success - Set hardware_acceleration");
	} else {
		debug_log("ERROR - Set hardware_acceleration");
		return ret;
	}

	ret = mm_util_set_colorspace_convert(MMHandle, MM_UTIL_IMG_FMT_RGB888);
	if(ret == MM_ERROR_NONE) {
		debug_log("Success - Set Convert Info");
	} else {
		media_format_unref(fmt);
		debug_log("ERROR - Set Convert Info");
		return ret;
	}

	ret = mm_util_set_resolution(MMHandle, 320, 240);
	if(ret == MM_ERROR_NONE) {
		debug_log("Success - Set Resize Info");
	} else {
		media_format_unref(fmt);
		debug_log("ERROR - Set Resize Info");
		return ret;
	}

	/* Transform */
	ret = mm_util_transform(MMHandle, src_packet, (mm_util_completed_callback) transform_completed_cb, handle);
	if(ret == MM_ERROR_NONE) {
		debug_log("Success - Transform");
	} else {
		media_format_unref(fmt);
		debug_error("ERROR - Transform");
		return ret;
	}

	debug_log("Wait...");

	media_packet_destroy(src_packet);
	media_format_unref(fmt);
	debug_log("destroy");

	return ret;
}


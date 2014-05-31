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

#ifndef __MM_UTILITY_IMGP_H__
#define __MM_UTILITY_IMGP_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
    @addtogroup UTILITY
    @{

    @par
    This part describes the APIs with repect to image converting library.
 */

/**
 * Image formats
 */
typedef enum
{
	/* YUV planar format */
	MM_UTIL_IMG_FMT_YUV420 = 0x00,  /**< YUV420 format - planer YV12*/
	MM_UTIL_IMG_FMT_YUV422,         /**< YUV422 format - planer */
	MM_UTIL_IMG_FMT_I420,           /**< YUV420 format - planar */
	MM_UTIL_IMG_FMT_NV12,           /**< NV12 format - planer */

	/* YUV packed format */
	MM_UTIL_IMG_FMT_UYVY,           /**< UYVY format - YUV packed format */
	MM_UTIL_IMG_FMT_YUYV,           /**< YUYV format - YUV packed format */

	/* RGB color */
	MM_UTIL_IMG_FMT_RGB565,         /**< RGB565 pixel format */
	MM_UTIL_IMG_FMT_RGB888,         /**< RGB888 pixel format */
	MM_UTIL_IMG_FMT_ARGB8888,       /**< ARGB8888 pixel format */

	MM_UTIL_IMG_FMT_BGRA8888,      /**< BGRA8888 pixel format */
	MM_UTIL_IMG_FMT_RGBA8888,      /**< RGBA8888 pixel format */
	MM_UTIL_IMG_FMT_BGRX8888,      /**<BGRX8888 pixel format */
	/* non-standard format */
	MM_UTIL_IMG_FMT_NV12_TILED,     /**< Customized color format in s5pc110 */
	MM_UTIL_IMG_FMT_NUM,            /**< Number of image formats */
} mm_util_img_format;


/**
 * Image rotation types
 */
typedef enum
{
	MM_UTIL_ROTATE_0,               /**< Rotation 0 degree - no effect */
	MM_UTIL_ROTATE_90,              /**< Rotation 90 degree */
	MM_UTIL_ROTATE_180,             /**< Rotation 180 degree */
	MM_UTIL_ROTATE_270,             /**< Rotation 270 degree */
	MM_UTIL_ROTATE_FLIP_HORZ,       /**< Flip horizontal */
	MM_UTIL_ROTATE_FLIP_VERT,       /**< Flip vertial */
	MM_UTIL_ROTATE_NUM              /**< Number of rotation types */
} mm_util_img_rotate_type;

/**
 * This function calculates the size of the image data in bytes.
 *
 * @param	format [in]     image format information
 * @param	width  [in]     pixel size of width
 * @param	height [in]     pixel size of height
 * @param	size   [out]    data size of image in bytes
 *
 * @return      This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_img_format
 * @since       R1, 1.0
 */
int
mm_util_get_image_size(mm_util_img_format format, unsigned int width, unsigned int height, unsigned int *size);


/**
 * This function convert the pixel format from source format to destination format.
 *
 * @param	src         [in]        pointer of source image data
 * @param	src_width   [in]        pixel size of source image width
 * @param	src_height  [in]        pixel size of source image height
 * @param	src_format  [in]        pixel format of source image
 * @param	dst         [in/out]    pointer of destination image data
 * @param	dst_format  [in]        pixel format of destination image
 *
 * @return      This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_img_format
 * @since       R1, 1.0
 */
int
mm_util_convert_colorspace(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
                           unsigned char *dst, mm_util_img_format dst_format);


/**
 * This function resizes the source image.
 *
 * @param	src         [in]        pointer of source image data
 * @param	src_width   [in]        pixel size of source image width
 * @param	src_height  [in]        pixel size of source image height
 * @param	src_format  [in]        pixel format of source image
 * @param	dst         [in/out]    pointer of destination image data
 * @param	dst_width   [in/out]    pixel size of destination image width
 * @param	dst_height  [in/out]    pixel size of destination image height
 *
 * @return      This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_img_format
 * @since       R1, 1.0
 */
int
mm_util_resize_image(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
                     unsigned char *dst, unsigned int *dst_width, unsigned int *dst_height);


/**
 * This function rotates the source image.
 *
 * @param	src         [in]        pointer of source image data
 * @param	src_width   [in]        pixel size of source image width
 * @param	src_height  [in]        pixel size of source image height
 * @param	src_format  [in]        pixel format of source image
 * @param	dst         [in/out]    pointer of destination image data
 * @param	dst_width   [in/out]    pixel size of destination image width
 * @param	dst_height  [in/out]    pixel size of destination image height
 * @param	angle       [in]        rotation enum value
 *
 * @return      This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_img_format, mm_util_img_rotate_type
 * @since       R1, 1.0
 */
int
mm_util_rotate_image(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
                     unsigned char *dst, unsigned int *dst_width, unsigned int *dst_height, mm_util_img_rotate_type angle);

/**
 * This function crop the source image.
 *
 * @param	src         [in]                pointer of source image data
 * @param	src_width   [in]             pixel size of source image width
 * @param	src_height  [in]             pixel size of source image height
 * @param	src_format  [in]            pixel format of source image
 * @param	crop_start_x   [in]         pixel point of cropped image
 * @param	crop_start_y  [in]          pixel point of cropped image
 * @param	crop_dest_width   [in/out]   cropped image width (value is changed when yuv image width is odd)
 * @param	crop_dest_height  [in/out]   cropped image height

 * @param	dst         [in/out]    pointer of destination image data
 *
 * @return      This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_img_format, mm_util_img_rotate_type
 * @since       R1, 1.0
 */
int
mm_util_crop_image(unsigned char *src, unsigned int src_width, unsigned int src_height, mm_util_img_format src_format,
                     unsigned int crop_start_x, unsigned int crop_start_y, unsigned int *crop_dest_width, unsigned int *crop_dest_height, unsigned char *dst);

#ifdef __cplusplus
}
#endif

#endif	/*__MM_UTILITY_IMGP_H__*/


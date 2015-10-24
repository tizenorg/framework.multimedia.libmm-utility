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

#ifndef __MM_UTIL_JPEG_H__
#define __MM_UTIL_JPEG_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
    @addtogroup UTILITY
    @{

    @par
    This part describes the APIs with repect to multimedia image library.
*/

/**
 * YUV format for jpeg
 */
typedef enum
{
	/* YUV planar format */
	MM_UTIL_JPEG_FMT_YUV420 = 0x00,  /**< YUV420 format - planer I420 */
	MM_UTIL_JPEG_FMT_YUV422,         /**< YUV422 format - planer */
	/* YUV packed format */
	MM_UTIL_JPEG_FMT_UYVY,            /**< UYVY format - YUV packed format */
	/* RGB888 format */
	MM_UTIL_JPEG_FMT_RGB888,	     /**< RGB888 format */
	/* GrayScale format */
	MM_UTIL_JPEG_FMT_GraySacle,    /**< GrayScale format */
	/* YUV packed format */
	MM_UTIL_JPEG_FMT_YUYV,            /**< YUYV format - YUV packed format */
	/*Following formats are only for qualcomm hardware codecs.*/
	MM_UTIL_JPEG_FMT_NV12,             /**< YUV format - YUV420 semi-planar (NV12) */
	MM_UTIL_JPEG_FMT_NV21,             /**< YUV format - YUV420 semi-planar (NV21) */
	MM_UTIL_JPEG_FMT_NV16,             /**< YUV format - YUV422 semi-planar (NV16) */
	MM_UTIL_JPEG_FMT_NV61,             /**< YUV format - YUV422 semi-planar (NV61) */
	MM_UTIL_JPEG_FMT_RGBA8888,     /**< RGBA8888 format */
	MM_UTIL_JPEG_FMT_BGRA8888,     /**< BGRA8888 format */
	MM_UTIL_JPEG_FMT_ARGB8888,     /**< ARGB8888 format */
} mm_util_jpeg_yuv_format;

/**
 * downscale decoding for jpeg
 */
typedef enum
{
	MM_UTIL_JPEG_DECODE_DOWNSCALE_1_1 = 1,	/** 1/1 downscale decode */
	MM_UTIL_JPEG_DECODE_DOWNSCALE_1_2 = 2,	/** 1/2 downscale decode */
	MM_UTIL_JPEG_DECODE_DOWNSCALE_1_4 = 4,	/** 1/4 downscale decode */
	MM_UTIL_JPEG_DECODE_DOWNSCALE_1_8 = 8,	/** 1/8 downscale decode */
} mm_util_jpeg_decode_downscale;

/**
 * YUV data
 */
typedef struct
{
	mm_util_jpeg_yuv_format format;     /**< pixel format*/
	int width;                          /**< width */
	int height;                         /**< heigt */
	int size;                           /**< size */
	void *data;                         /**< data */ //    int decode_yuv_subsample;                    /**< decode_yuv_subsample */
} mm_util_jpeg_yuv_data;


/**
 * This function encodes rawfile to jpeg file.
 *
 * @param filename [in]     output file name
 * @param src [in]          pointer of input stream pointer (raw data)
 * @param width [in]        width of src data
 * @param height [in]       height of src data
 * @param fmt [in]          color format
 * @param quality [in]      encoding quality, from 1 to 100
 * @return This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_jpeg_yuv_format
 * @since       R1, 1.0
 */
int
mm_util_jpeg_encode_to_file (const char *filename, void *src, int width, int height, mm_util_jpeg_yuv_format fmt, int quality);


/**
 * This function encodes raw file to jpeg into memory.
 *
 * @param mem [out]         pointer of output stream pointer, that is, pointer of encoded jpeg stream pointer.
                            After using it, please free the allocated memory.
 * @param size [in]         output stream size, that is, encoded stream size
 * @param src [in]          pointer of input stream pointer (raw data)
 * @param width [in]        width of src data
 * @param height [in]       height of src data
 * @param fmt [in]          color format
 * @param quality [in]      encoding quality, from 1 to 100
 * @return This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_jpeg_yuv_format
 * @since       R1, 1.0
 */
int
mm_util_jpeg_encode_to_memory (void **mem, int *size, void *src, int width, int height, mm_util_jpeg_yuv_format fmt, int quality);


/**
 * This function extracts yuv data from jpeg file
 *
 * @param decoded [out]     pointer of output stream pointer, that is, pointer of encoded jpeg stream pointer.
                            After using it, please free the allocated memory.
 * @param filename [in]     input file name, encoded stream file
 * @param fmt [in]          color format
 * @return This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_jpeg_yuv_data, mm_util_jpeg_yuv_format
 * @since       R1, 1.0
 */
int
mm_util_decode_from_jpeg_file(mm_util_jpeg_yuv_data *decoded, const char *filename, mm_util_jpeg_yuv_format fmt);


/**
 * This function extracts yuv data from jpeg buffer
 *
 * @param decoded [out]     pointer of output stream pointer, that is, pointer of encoded jpeg stream pointer.
                            After using it, please free the allocated memory.
 * @param src [in]          input stream pointer(pointer of encoded jpeg stream data)
 * @param size [in]         size of input stream(size of pointer of encoded jpeg stream data)
 * @param fmt [in]          color format
 * @return This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_jpeg_yuv_data, mm_util_jpeg_yuv_format
 * @since       R1, 1.0
 */
int
mm_util_decode_from_jpeg_memory (mm_util_jpeg_yuv_data *decoded, void *src, int size, mm_util_jpeg_yuv_format fmt);

/**
 * This function extracts yuv data from jpeg file with downscale decode option
 *
 * @param decoded [out]     pointer of output stream pointer, that is, pointer of encoded jpeg stream pointer.
                            After using it, please free the allocated memory.
 * @param filename [in]     input file name, encoded stream file
 * @param fmt [in]          color format
 * @param downscale [in]       downscale value
 * @return This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_jpeg_yuv_data, mm_util_jpeg_yuv_format
 * @since       R1, 1.0
 */
int
mm_util_decode_from_jpeg_file_with_downscale(mm_util_jpeg_yuv_data *decoded, const char *filename, mm_util_jpeg_yuv_format fmt, mm_util_jpeg_decode_downscale downscale);

/**
 * This function extracts yuv data from jpeg buffer with downscale decode option
 *
 * @param decoded [out]     pointer of output stream pointer, that is, pointer of encoded jpeg stream pointer.
                            After using it, please free the allocated memory.
 * @param src [in]          input stream pointer(pointer of encoded jpeg stream data)
 * @param size [in]         size of input stream(size of pointer of encoded jpeg stream data)
 * @param fmt [in]          color format
 * @param downscale [in]       downscale value
 * @return This function returns zero on success, or negative value with error code.
 * @remark
 * @see         mm_util_jpeg_yuv_data, mm_util_jpeg_yuv_format
 * @since       R1, 1.0
 */
int
mm_util_decode_from_jpeg_memory_with_downscale (mm_util_jpeg_yuv_data *decoded, void *src, int size, mm_util_jpeg_yuv_format fmt, mm_util_jpeg_decode_downscale downscale);


#ifdef __cplusplus
}
#endif

#endif    /*__MM_UTIL_JPEG_H__*/


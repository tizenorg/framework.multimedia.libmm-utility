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
 *ranklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <mm_debug.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h> /* fopen() */
#include <jpeglib.h>
#define LIBJPEG_TURBO 0
#define ENC_MAX_LEN 8192
#if LIBJPEG_TURBO
#include <turbojpeg.h>
#endif
#include <mm_error.h>
#include <setjmp.h>
#include <glib.h>
#include <mm_attrs.h>
#include <mm_attrs_private.h>
#include <mm_ta.h>
#include "mm_util_jpeg.h"
#include "mm_util_imgp.h"
#ifndef YUV420_SIZE
#define YUV420_SIZE(width, height)	(width*height*3>>1)
#endif
#ifndef YUV422_SIZE
#define YUV422_SIZE(width, height)	(width*height<<1)
#endif

/* H/W JPEG codec */
#include <dlfcn.h>
#define ENV_NAME_USE_HW_CODEC           "IMAGE_UTIL_USE_HW_CODEC"
#define LIB_PATH_HW_CODEC_LIBRARY       "/usr/lib/libmm_jpeg_hw.so"
#define ENCODE_JPEG_HW_FUNC_NAME        "mm_jpeg_encode_hw"
#define DECODE_JPEG_HW_FUNC_NAME        "mm_jpeg_decode_hw"
typedef int (*EncodeJPEGFunc)(unsigned char *src, int width, int height, mm_util_jpeg_yuv_format in_fmt, int quality,
                              unsigned char **dst, int *dst_size);
typedef int (*DecodeJPEGFunc)(unsigned char *src, int src_size, mm_util_jpeg_yuv_format out_fmt,
                              unsigned char **dst, int *dst_width, int *dst_height, int *dst_size);
static int _read_file(char *file_name, void **data, int *data_size);
static int _write_file(char *file_name, void *data, int data_size);

#define PARTIAL_DECODE 0
#define LIBJPEG 1
#define MM_JPEG_ROUND_UP_2(num)  (((num)+1)&~1)
#define MM_JPEG_ROUND_UP_4(num)  (((num)+3)&~3)
#define MM_JPEG_ROUND_UP_8(num)  (((num)+7)&~7)
#define MM_JPEG_ROUND_DOWN_16(num) ((num)&(~15))
#if LIBJPEG_TURBO
#define PAD(v, p) ((v+(p)-1)&(~((p)-1)))
#define _throwtj() {printf("TurboJPEG ERROR:\n%s\n", tjGetErrorStr());}
#define _tj(f) {if((f)==-1) _throwtj();}

const char *subName[TJ_NUMSAMP]={"444", "422", "420", "GRAY", "440"};

void initBuf(unsigned char *buf, int w, int h, int pf, int flags)
{
	int roffset=tjRedOffset[pf];
	int goffset=tjGreenOffset[pf];
	int boffset=tjBlueOffset[pf];
	int ps=tjPixelSize[pf];
	int index, row, col, halfway=16;

	memset(buf, 0, w*h*ps);
	if(pf==TJPF_GRAY) {
		for(row=0; row<h; row++) {
			for(col=0; col<w; col++) {
				if(flags&TJFLAG_BOTTOMUP) index=(h-row-1)*w+col;
				else index=row*w+col;
				if(((row/8)+(col/8))%2==0) buf[index]=(row<halfway)? 255:0;
				else buf[index]=(row<halfway)? 76:226;
			}
		}
	} else {
		for(row=0; row<h; row++) {
			for(col=0; col<w; col++) {
				if(flags&TJFLAG_BOTTOMUP) index=(h-row-1)*w+col;
				else index=row*w+col;
				if(((row/8)+(col/8))%2==0) {
					if(row<halfway) {
						buf[index*ps+roffset]=255;
						buf[index*ps+goffset]=255;
						buf[index*ps+boffset]=255;
					}
				} else {
					buf[index*ps+roffset]=255;
					if(row>=halfway) buf[index*ps+goffset]=255;
				}
			}
		}
	}
}

static void
mm_image_codec_writeJPEG(unsigned char *jpegBuf, unsigned long jpegSize, char *filename)
{
	FILE *file=fopen(filename, "wb");
	if(!file || fwrite(jpegBuf, jpegSize, 1, file)!=1) {
		debug_error("ERROR: Could not write to %s \t %s", filename, strerror(errno));
	}

	if(file) fclose(file);
}

static void
_mm_decode_libjpeg_turbo_decompress(tjhandle handle, unsigned char *jpegBuf, unsigned long jpegSize, int TD_BU,mm_util_jpeg_yuv_data* decoded_data, mm_util_jpeg_yuv_format input_fmt)
{
	int _hdrw=0, _hdrh=0, _hdrsubsamp=-1;
	int scaledWidth=0;
	int scaledHeight=0;
	unsigned long dstSize=0;
	int i=0,n=0;
	tjscalingfactor _sf;
	tjscalingfactor*sf=tjGetScalingFactors(&n), sf1={1, 1};

	if(jpegBuf == NULL) {
		debug_error("jpegBuf is NULL");
		return;
	}

	debug_log("0x%2x, 0x%2x ", jpegBuf[0], jpegBuf[1]);

	_tj(tjDecompressHeader2(handle, jpegBuf, jpegSize, &_hdrw, &_hdrh, &_hdrsubsamp));

	if(!sf || !n) {
		debug_error(" scaledfactor is NULL");
		return;
	}

	if((_hdrsubsamp==TJSAMP_444 || _hdrsubsamp==TJSAMP_GRAY) || input_fmt==MM_UTIL_JPEG_FMT_RGB888) {
		_sf= sf[i];
	} else {
		_sf=sf1;
	}

	scaledWidth=TJSCALED(_hdrw, _sf);
	scaledHeight=TJSCALED(_hdrh, _sf);
	debug_log("_hdrw:%d _hdrh:%d, _hdrsubsamp:%d, scaledWidth:%d, scaledHeight:%d",  _hdrw, _hdrh, _hdrsubsamp, scaledWidth, scaledHeight);

	if(input_fmt==MM_UTIL_JPEG_FMT_YUV420 || input_fmt==MM_UTIL_JPEG_FMT_YUV422) {
		debug_log("JPEG -> YUV %s ... ", subName[_hdrsubsamp]);
	} else if(input_fmt==MM_UTIL_JPEG_FMT_RGB888) {
		debug_log("JPEG -> RGB %d/%d ... ", _sf.num, _sf.denom);
	}

	if(input_fmt==MM_UTIL_JPEG_FMT_YUV420 || input_fmt==MM_UTIL_JPEG_FMT_YUV422) {
		dstSize=TJBUFSIZEYUV(_hdrw, _hdrh, _hdrsubsamp);
		debug_log("MM_UTIL_JPEG_FMT_YUV420 dstSize: %d  _hdrsubsamp:%d TJBUFSIZEYUV(w, h, _hdrsubsamp): %d",
			dstSize, _hdrsubsamp, TJBUFSIZEYUV(_hdrw, _hdrh, _hdrsubsamp));
	}
	else if(input_fmt==MM_UTIL_JPEG_FMT_RGB888)
	{
		dstSize=scaledWidth*scaledHeight*tjPixelSize[TJPF_RGB];
		debug_log("MM_UTIL_JPEG_FMT_RGB888 dstSize: %d _hdrsubsamp:%d", dstSize, _hdrsubsamp);
	}

	if((decoded_data->data=(void*)malloc(dstSize))==NULL) {
		debug_error("dstBuf is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	debug_log("dstBuf:%p", decoded_data->data);

	if(input_fmt==MM_UTIL_JPEG_FMT_YUV420 || input_fmt==MM_UTIL_JPEG_FMT_YUV422) {
		_tj(tjDecompressToYUV(handle, jpegBuf, jpegSize, decoded_data->data, TD_BU));
		debug_log("MM_UTIL_JPEG_FMT_YUV420, dstBuf: %d", decoded_data->data);
		decoded_data->format=input_fmt;
	}
	else if(input_fmt==MM_UTIL_JPEG_FMT_RGB888) {
		_tj(tjDecompress2(handle, jpegBuf, jpegSize, decoded_data->data, scaledWidth, 0, scaledHeight, TJPF_RGB, TD_BU));
		debug_log("MM_UTIL_JPEG_FMT_RGB888, dstBuf: %p", decoded_data->data);
		decoded_data->format=MM_UTIL_JPEG_FMT_RGB888;
	} else {
		debug_error("[%s][%05d] We can't support the IMAGE format");
		return;
	}

	decoded_data->size=dstSize;
	if(input_fmt==MM_UTIL_JPEG_FMT_RGB888)
	{
		decoded_data->width=scaledWidth;
		decoded_data->height=scaledHeight;
		decoded_data->size = dstSize;
	}
	else if(input_fmt==MM_UTIL_JPEG_FMT_YUV420 ||input_fmt==MM_UTIL_JPEG_FMT_YUV422)
	{
		if(_hdrsubsamp == TJSAMP_422) {
			debug_log("_hdrsubsamp == TJSAMP_422", decoded_data->width, _hdrw);
			decoded_data->width=MM_JPEG_ROUND_UP_2(_hdrw);
		} else {
			if(_hdrw% 4 != 0) {
				decoded_data->width=MM_JPEG_ROUND_UP_4(_hdrw);
			} else {
				decoded_data->width=_hdrw;
			}
		}

		if(_hdrsubsamp == TJSAMP_420) {
			debug_log("_hdrsubsamp == TJSAMP_420", decoded_data->width, _hdrw);
			if(_hdrh% 4 != 0) {
				decoded_data->height=MM_JPEG_ROUND_UP_4(_hdrh);
			} else {
			decoded_data->height=_hdrh;
			}
		} else {
			decoded_data->height=_hdrh;
		}
		decoded_data->size = dstSize;
	}
}

static int
mm_image_decode_from_jpeg_file_with_libjpeg_turbo(mm_util_jpeg_yuv_data * decoded_data, char *pFileName, mm_util_jpeg_yuv_format input_fmt)
{
	int iErrorCode	= MM_ERROR_NONE;
	tjhandle dhandle=NULL;
	unsigned char *srcBuf=NULL;
	int jpegSize;
	int TD_BU=0;

	void *src = fopen(pFileName, "rb" );
	if(src == NULL) {
		debug_error("Error");
		return MM_ERROR_IMAGE_FILEOPEN;
	}
	fseek (src, 0,  SEEK_END);
	jpegSize = ftell(src);
	rewind(src);

	if((dhandle=tjInitDecompress())==NULL) {
		fclose(src);
		debug_error("dhandle=tjInitDecompress()) is NULL");
		return MM_ERROR_IMAGEHANDLE_NOT_INITIALIZED;
	}
	srcBuf=(unsigned char *)malloc(sizeof(char) * jpegSize);
	if(srcBuf==NULL) {
		fclose(src);
		tjDestroy(dhandle);
		debug_error("srcBuf is NULL");
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}

	fread(srcBuf,1, jpegSize, src);
	debug_log("srcBuf[0]: 0x%2x, srcBuf[1]: 0x%2x, jpegSize:%d", srcBuf[0], srcBuf[1], jpegSize);
	_mm_decode_libjpeg_turbo_decompress(dhandle, srcBuf, jpegSize, TD_BU, decoded_data, input_fmt);
	fclose(src);
	debug_log("fclose");

	if(dhandle) {
		tjDestroy(dhandle);
	}
	if(srcBuf) {
		tjFree(srcBuf);
	}

	return iErrorCode;
}

static int
mm_image_decode_from_jpeg_memory_with_libjpeg_turbo(mm_util_jpeg_yuv_data * decoded_data, void *src, int size, mm_util_jpeg_yuv_format input_fmt)
{
	int iErrorCode	= MM_ERROR_NONE;
	tjhandle dhandle=NULL;
	unsigned char *srcBuf=NULL;
	int TD_BU=0;

	if((dhandle=tjInitDecompress())==NULL) {
		debug_error("dhandle=tjInitDecompress()) is NULL");
		return MM_ERROR_IMAGEHANDLE_NOT_INITIALIZED;
	}
	srcBuf=(unsigned char *)malloc(sizeof(char) * size);
	if(srcBuf==NULL) {
		fclose(src);
		tjDestroy(dhandle);
		debug_error("srcBuf is NULL");
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}

	debug_log("srcBuf[0]: 0x%2x, srcBuf[1]: 0x%2x, jpegSize:%d", srcBuf[0], srcBuf[1], size);
	_mm_decode_libjpeg_turbo_decompress(dhandle, src, size, TD_BU, decoded_data, input_fmt);

	if(dhandle) {
		tjDestroy(dhandle);
	}
	if(srcBuf) {
		tjFree(srcBuf);
	}

	return iErrorCode;
}

static void
_mm_encode_libjpeg_turbo_compress(tjhandle handle, void *src, unsigned char **dstBuf, unsigned long *dstSize, int w, int h, int jpegQual, int flags, mm_util_jpeg_yuv_format fmt)
{
	unsigned char *srcBuf=NULL;
	unsigned long jpegSize=0;

	jpegSize=w*h*tjPixelSize[TJPF_RGB];
	srcBuf=(unsigned char *)malloc(jpegSize);

	if(srcBuf==NULL) {
		debug_error("srcBuf is NULL");
		return;
	} else {
		debug_log("srcBuf: 0x%2x", srcBuf);
		if(fmt==MM_UTIL_JPEG_FMT_RGB888) {
			initBuf(srcBuf, w, h, TJPF_RGB, flags);
		} else if(fmt==MM_UTIL_JPEG_FMT_YUV420 || fmt==MM_UTIL_JPEG_FMT_YUV422) {
			initBuf(srcBuf, w, h, TJPF_GRAY, flags);
		} else {
			free(srcBuf);
			debug_error("[%s][%05d] We can't support the IMAGE format");
			return;
		}
		memcpy(srcBuf, src, jpegSize);
	}

	if(*dstBuf && *dstSize>0) memset(*dstBuf, 0, *dstSize);
	debug_log("Done.");
	*dstSize=TJBUFSIZE(w, h);
	if(fmt==MM_UTIL_JPEG_FMT_RGB888) {
		_tj(tjCompress2(handle, srcBuf, w, 0, h, TJPF_RGB, dstBuf, dstSize, TJPF_RGB, jpegQual, flags));
		debug_log("*dstSize: %d", *dstSize);
	} else if(fmt==MM_UTIL_JPEG_FMT_YUV420) {
		*dstSize=TJBUFSIZEYUV(w, h, TJSAMP_420);
		_tj(tjEncodeYUV2(handle, srcBuf, w, 0, h, TJPF_RGB, *dstBuf, TJSAMP_420, flags));
		debug_log("*dstSize: %d \t decode_yuv_subsample: %d TJBUFSIZE(w, h):%d", *dstSize, TJSAMP_420, TJBUFSIZE(w, h));
	} else if(fmt==MM_UTIL_JPEG_FMT_YUV422) {
		*dstSize=TJBUFSIZEYUV(w, h, TJSAMP_422);
		_tj(tjEncodeYUV2(handle, srcBuf, w, 0, h, TJPF_RGB, *dstBuf, TJSAMP_422, flags));
		debug_log("*dstSize: %d \t decode_yuv_subsample: %d TJBUFSIZE(w, h):%d", *dstSize, TJSAMP_422, TJBUFSIZE(w, h));
	} else {
		debug_error("fmt:%d  is wrong", fmt);
	}
	if(srcBuf) {
		free(srcBuf);
	}
}

static int
mm_image_encode_to_jpeg_file_with_libjpeg_turbo(char *filename, void* src, int width, int height, mm_util_jpeg_yuv_format fmt, int quality)
{
	int iErrorCode	= MM_ERROR_NONE;
	tjhandle chandle=NULL;
	unsigned char *dstBuf=NULL;
	unsigned long size=0;
	int TD_BU=0;
	FILE *fout=fopen(filename, "w+");
	if(fout == NULL) {
		debug_error("FILE OPEN FAIL");
		return MM_ERROR_IMAGE_FILEOPEN;
	}
	debug_log("fmt: %d", fmt);

	size=TJBUFSIZE(width, height);
	if((dstBuf=(unsigned char *)tjAlloc(size))==NULL) {
		fclose(fout);
		debug_error("dstBuf is NULL");
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}

	if((chandle=tjInitCompress())==NULL) {
		fclose(fout);
		tjFree(dstBuf);
		debug_error("dstBuf is NULL");
		return MM_ERROR_IMAGEHANDLE_NOT_INITIALIZED;
	}
	_mm_encode_libjpeg_turbo_compress(chandle, src, &dstBuf, &size, width, height, quality, TD_BU, fmt);
	debug_log("dstBuf: %p\t size: %d", dstBuf, size);
	fwrite(dstBuf, 1, size, fout);
	fclose(fout);
	if(chandle) {
		tjDestroy(chandle);
	}
	if(dstBuf) {
		tjFree(dstBuf);
	}
	debug_log("Done");
	return iErrorCode;
}

static int
mm_image_encode_to_jpeg_memory_with_libjpeg_turbo(void **mem, int *csize, void *rawdata,  int w, int h, mm_util_jpeg_yuv_format fmt,int quality)
{
	int iErrorCode	= MM_ERROR_NONE;
	tjhandle chandle=NULL;
	int TD_BU=0;

	*csize=TJBUFSIZE(w, h);
	if(fmt==MM_UTIL_JPEG_FMT_RGB888) {
		debug_log("MM_UTIL_JPEG_FMT_RGB888 size: %d", *csize);
	}else if(fmt==MM_UTIL_JPEG_FMT_YUV420 ||fmt==MM_UTIL_JPEG_FMT_YUV422) {
		debug_log("TJSAMP_420 ||TJSAMP_422 size: %d",*csize);
	}else {
			debug_error("We can't support the IMAGE format");
			return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	if((*mem=(unsigned char *)tjAlloc(*csize))==NULL) {
		debug_error("dstBuf is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if((chandle=tjInitCompress())==NULL) {
		tjFree(*mem);
		debug_error("chandle is NULL");
		return MM_ERROR_IMAGEHANDLE_NOT_INITIALIZED;
	}

	debug_log("width: %d height: %d, size: %d", w, h, *csize);
	_mm_encode_libjpeg_turbo_compress(chandle, rawdata, mem, csize, w, h, quality, TD_BU, fmt);
	debug_log("dstBuf: %p &dstBuf:%p size: %d", *mem, mem, *csize);
	if(chandle) {
		tjDestroy(chandle);
	}
	return iErrorCode;
}
#endif

/* OPEN SOURCE */
struct my_error_mgr_s
{
	struct jpeg_error_mgr pub; /* "public" fields */
	jmp_buf setjmp_buffer; /* for return to caller */
} my_error_mgr_s;

typedef struct my_error_mgr_s * my_error_ptr;

struct {
	struct jpeg_destination_mgr pub; /* public fields */
	FILE * outfile; /* target stream */
	JOCTET * buffer; /* start of buffer */
} my_destination_mgr_s;

typedef struct  my_destination_mgr_s * my_dest_ptr;

#define OUTPUT_BUF_SIZE  4096	/* choose an efficiently fwrite'able size */

/* Expanded data destination object for memory output */

struct {
	struct jpeg_destination_mgr pub; /* public fields */

	unsigned char ** outbuffer; /* target buffer */
	unsigned long * outsize;
	unsigned char * newbuffer; /* newly allocated buffer */
	JOCTET * buffer; /* start of buffer */
	size_t bufsize;
} my_mem_destination_mgr;

typedef struct my_mem_destination_mgr * my_mem_dest_ptr;

static void
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err; /* cinfo->err really points to a my_error_mgr_s struct, so coerce pointer */
	(*cinfo->err->output_message) (cinfo); /*  Always display the message. We could postpone this until after returning, if we chose. */
	longjmp(myerr->setjmp_buffer, 1); /*  Return control to the setjmp point */
}

static int
mm_image_encode_to_jpeg_file_with_libjpeg(char *pFileName, void *rawdata, int width, int height, mm_util_jpeg_yuv_format fmt, int quality)
{
	int iErrorCode = MM_ERROR_NONE;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int i, j, flag, _height;
	FILE * fpWriter;

	JSAMPROW y[16],cb[16],cr[16]; /* y[2][5] = color sample of row 2 and pixel column 5; (one plane) */
	JSAMPARRAY data[3]; /* t[0][2][5] = color sample 0 of row 2 and column 5 */
	if(!pFileName) {
		debug_error("pFileName");
		return MM_ERROR_IMAGE_FILEOPEN;
	}

	data[0] = y;
	data[1] = cb;
	data[2] = cr;

	debug_log("Enter");

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);

	if ((fpWriter = fopen(pFileName, "wb")) == NULL) {
		debug_error("Exit on error");
		return MM_ERROR_IMAGE_FILEOPEN;
	}

	jpeg_stdio_dest(&cinfo, fpWriter);
	cinfo.image_width = width;
	cinfo.image_height = height;
	if (fmt ==MM_UTIL_JPEG_FMT_YUV420 ||fmt ==MM_UTIL_JPEG_FMT_YUV422 || fmt ==MM_UTIL_JPEG_FMT_UYVY) {
		debug_log("[mm_image_encode_to_jpeg_file_with_libjpeg] jpeg_stdio_dest for YUV");
		debug_log("[Height] %d", height);
		_height = MM_JPEG_ROUND_DOWN_16(height);
		flag = height - _height;

		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_YCbCr;
		debug_log("JCS_YCbCr [%d] ", flag);

		jpeg_set_defaults(&cinfo);
		debug_log("jpeg_set_defaults");

		cinfo.raw_data_in = TRUE; /*  Supply downsampled data */
		cinfo.do_fancy_downsampling = FALSE;

		cinfo.comp_info[0].h_samp_factor = 2;
		if (fmt ==MM_UTIL_JPEG_FMT_YUV420) {
			cinfo.comp_info[0].v_samp_factor = 2;
		} else if (fmt ==MM_UTIL_JPEG_FMT_YUV422 || fmt ==MM_UTIL_JPEG_FMT_UYVY) {
			cinfo.comp_info[0].v_samp_factor = 1;
		}
		cinfo.comp_info[1].h_samp_factor = 1;
		cinfo.comp_info[1].v_samp_factor = 1;
		cinfo.comp_info[2].h_samp_factor = 1;
		cinfo.comp_info[2].v_samp_factor = 1;

		jpeg_set_quality(&cinfo, quality, TRUE);
		debug_log("jpeg_set_quality");
		cinfo.dct_method = JDCT_FASTEST;

		jpeg_start_compress(&cinfo, TRUE);
		debug_log("jpeg_start_compress");

		if(flag) {
			for (j = 0; j < _height; j += 16) {
				for (i = 0; i < 16; i++) {
					y[i] = (JSAMPROW)rawdata + width * (i + j);
					if (i % 2 == 0) {
						cb[i / 2] = (JSAMPROW)rawdata + width * height + width / 2 * ((i + j) / 2);
						cr[i / 2] = (JSAMPROW)rawdata + width * height + width * height / 4 + width / 2 * ((i + j) / 2);
					}
				}
				jpeg_write_raw_data(&cinfo, data, 16);
			}
			for (i = 0; i < 16; i++) {
				y[i] = (JSAMPROW)rawdata + width * (i + j+flag-1);
				if (i % 2 == 0) {
					cb[i / 2] = (JSAMPROW)rawdata + width * height + width / 2 * ((i + j+flag-1) / 2);
					cr[i / 2] = (JSAMPROW)rawdata + width * height + width * height / 4 + width / 2 * ((i + j+flag-1) / 2);
				}
				jpeg_write_raw_data(&cinfo, data, 16);
			}
		} else {
			for (j = 0; j < height; j += 16) {
				for (i = 0; i < 16; i++) {
					y[i] = (JSAMPROW)rawdata + width * (i + j);
					if (i % 2 == 0) {
						cb[i / 2] = (JSAMPROW)rawdata + width * height + width / 2 * ((i + j) / 2);
						cr[i / 2] = (JSAMPROW)rawdata + width * height + width * height / 4 + width / 2 * ((i + j) / 2);
					}
				}
				jpeg_write_raw_data(&cinfo, data, 16);
			}
		}
		debug_log("#for loop#");

		jpeg_finish_compress(&cinfo);
		debug_log("jpeg_finish_compress");

		jpeg_destroy_compress(&cinfo);
		debug_log("jpeg_destroy_compress");
	} else if (fmt == MM_UTIL_JPEG_FMT_RGB888 || fmt == MM_UTIL_JPEG_FMT_GraySacle) {
		JSAMPROW row_pointer[1];
		int iRowStride;

		if(fmt == MM_UTIL_JPEG_FMT_RGB888) {
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			debug_log("JCS_RGB");
		} else if(fmt == MM_UTIL_JPEG_FMT_GraySacle) {
			cinfo.input_components = 1; /* one colour component */
			cinfo.in_color_space = JCS_GRAYSCALE;
			debug_log("JCS_GRAYSCALE");
		}

		jpeg_set_defaults(&cinfo);
		debug_log("jpeg_set_defaults");
		jpeg_set_quality(&cinfo, quality, TRUE);
		debug_log("jpeg_set_quality");
		jpeg_start_compress(&cinfo, TRUE);
		debug_log("jpeg_start_compress");
		if(fmt == MM_UTIL_JPEG_FMT_RGB888) {
			iRowStride = width * 3;
		} else if(fmt == MM_UTIL_JPEG_FMT_GraySacle) {
			iRowStride = width;
		}

		while (cinfo.next_scanline < cinfo.image_height) {
			row_pointer[0] = &rawdata[cinfo.next_scanline * iRowStride];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}
		debug_log("while");
		jpeg_finish_compress(&cinfo);
		debug_log("jpeg_finish_compress");

		jpeg_destroy_compress(&cinfo);
		debug_log("jpeg_destroy_compress");
	} else {
		debug_error("We can't encode the IMAGE format");
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	sync();
	debug_log("[sync] FILE");
	fclose(fpWriter);
	return iErrorCode;
}

static int
mm_image_encode_to_jpeg_memory_with_libjpeg(void **mem, int *csize, void *rawdata, int width, int height, mm_util_jpeg_yuv_format fmt,int quality)
{
	int iErrorCode	= MM_ERROR_NONE;

	JSAMPROW y[16],cb[16],cr[16]; /*  y[2][5] = color sample of row 2 and pixel column 5; (one plane) */
	JSAMPARRAY data[3]; /*  t[0][2][5] = color sample 0 of row 2 and column 5 */

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int i, j, flag, _height;
	*csize = 0;
	data[0] = y;
	data[1] = cb;
	data[2] = cr;

	debug_log("Enter");
	debug_log("#Before Enter#, mem: %p\t rawdata:%p\t width: %d\t height: %d\t fmt: %d\t quality: %d"
		, mem, rawdata, width, height, fmt, quality);

	if(rawdata == NULL) {
		debug_error("Exit on error -rawdata");
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}

	if (mem == NULL) {
		debug_error("Exit on error -mem");
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}

	cinfo.err = jpeg_std_error(&jerr); /*  Errors get written to stderr */

	jpeg_create_compress(&cinfo);

	debug_log("[mm_image_encode_to_jpeg_memory_with_libjpeg] #After jpeg_mem_dest#, mem: %p\t width: %d\t height: %d\t fmt: %d\t quality: %d"
		, mem, width, height, fmt, quality);

	jpeg_mem_dest(&cinfo, (unsigned char **)mem, (unsigned long *)csize);
	cinfo.image_width = width;
	cinfo.image_height = height;
	if (fmt ==MM_UTIL_JPEG_FMT_YUV420 ||fmt ==MM_UTIL_JPEG_FMT_YUV422 || fmt ==MM_UTIL_JPEG_FMT_UYVY) {
		debug_log("[mm_image_encode_to_jpeg_file_with_libjpeg] jpeg_stdio_dest for YUV");
		_height = MM_JPEG_ROUND_DOWN_16(height);
		flag = height - _height;

		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_YCbCr;
		jpeg_set_defaults(&cinfo);
		debug_log("jpeg_set_defaults");

		cinfo.raw_data_in = TRUE; /* Supply downsampled data */
		cinfo.do_fancy_downsampling = FALSE;

		cinfo.comp_info[0].h_samp_factor = 2;
		if (fmt ==MM_UTIL_JPEG_FMT_YUV420) {
			cinfo.comp_info[0].v_samp_factor = 2;
		} else if (fmt ==MM_UTIL_JPEG_FMT_YUV422 || fmt ==MM_UTIL_JPEG_FMT_UYVY) {
			cinfo.comp_info[0].v_samp_factor = 1;
		}
		cinfo.comp_info[1].h_samp_factor = 1;
		cinfo.comp_info[1].v_samp_factor = 1;
		cinfo.comp_info[2].h_samp_factor = 1;
		cinfo.comp_info[2].v_samp_factor = 1;

		jpeg_set_quality(&cinfo, quality, TRUE);
		debug_log("jpeg_set_quality");
		cinfo.dct_method = JDCT_FASTEST;

		jpeg_start_compress(&cinfo, TRUE);
		debug_log("jpeg_start_compress");

		if(flag) {
			for (j = 0; j < _height; j += 16) {
				for (i = 0; i < 16; i++) {
					y[i] = (JSAMPROW)rawdata + width * (i + j);
					if (i % 2 == 0) {
						cb[i / 2] = (JSAMPROW)rawdata + width * height + width / 2 * ((i + j) / 2);
						cr[i / 2] = (JSAMPROW)rawdata + width * height + width * height / 4 + width / 2 * ((i + j) / 2);
					}
				}
				jpeg_write_raw_data(&cinfo, data, 16);
			}
			for (i = 0; i < 16; i++) {
				y[i] = (JSAMPROW)rawdata + width * (i + j+flag-1);
				if (i % 2 == 0) {
					cb[i / 2] = (JSAMPROW)rawdata + width * height + width / 2 * ((i + j+flag-1) / 2);
					cr[i / 2] = (JSAMPROW)rawdata + width * height + width * height / 4 + width / 2 * ((i + j+flag-1) / 2);
				}
				jpeg_write_raw_data(&cinfo, data, 16);
			}
		} else {
			for (j = 0; j < height; j += 16) {
				for (i = 0; i < 16; i++) {
					y[i] = (JSAMPROW)rawdata + width * (i + j);
					if (i % 2 == 0) {
						cb[i / 2] = (JSAMPROW)rawdata + width * height + width / 2 * ((i + j) / 2);
						cr[i / 2] = (JSAMPROW)rawdata + width * height + width * height / 4 + width / 2 * ((i + j) / 2);
					}
				}
				jpeg_write_raw_data(&cinfo, data, 16);
			}
		}
		debug_log("#for loop#");

		jpeg_finish_compress(&cinfo);
		debug_log("jpeg_finish_compress");
		jpeg_destroy_compress(&cinfo);
		debug_log("Exit jpeg_destroy_compress, mem: %p\t size:%d", *mem, *csize);
	}

	else if (fmt == MM_UTIL_JPEG_FMT_RGB888 ||fmt == MM_UTIL_JPEG_FMT_GraySacle) {
		JSAMPROW row_pointer[1];
		int iRowStride;

		debug_log("MM_UTIL_JPEG_FMT_RGB888");
		if(fmt == MM_UTIL_JPEG_FMT_RGB888) {
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			debug_log("JCS_RGB");
		} else if(fmt == MM_UTIL_JPEG_FMT_GraySacle) {
			cinfo.input_components = 1; /* one colour component */
			cinfo.in_color_space = JCS_GRAYSCALE;
			debug_log("JCS_GRAYSCALE");
		}

		jpeg_set_defaults(&cinfo);
		debug_log("jpeg_set_defaults");
		jpeg_set_quality(&cinfo, quality, TRUE);
		debug_log("jpeg_set_quality");
		jpeg_start_compress(&cinfo, TRUE);
		debug_log("jpeg_start_compress");
		if(fmt == MM_UTIL_JPEG_FMT_RGB888) {
			iRowStride = width * 3;
		} else if(fmt == MM_UTIL_JPEG_FMT_GraySacle) {
			iRowStride = width;
		}

		while (cinfo.next_scanline < cinfo.image_height) {
			row_pointer[0] = &rawdata[cinfo.next_scanline * iRowStride];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}
		debug_log("while");
		jpeg_finish_compress(&cinfo);
		debug_log("jpeg_finish_compress");

		jpeg_destroy_compress(&cinfo);

		debug_log("Exit");
	}	else {
		debug_error("We can't encode the IMAGE format");
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}
	return iErrorCode;
}

static int
mm_image_decode_from_jpeg_file_with_libjpeg(mm_util_jpeg_yuv_data * decoded_data, char *pFileName, mm_util_jpeg_yuv_format input_fmt)
{
	int iErrorCode	= MM_ERROR_NONE;
	FILE *infile		= NULL;
	struct jpeg_decompress_struct dinfo;
	struct my_error_mgr_s jerr;
	JSAMPARRAY buffer; /* Output row buffer */
	int row_stride = 0, state = 0; /* physical row width in output buffer */
	JSAMPROW image, u_image, v_image;
	JSAMPROW row; /* point to buffer[0] */

	debug_log("Enter");

	if(!pFileName) {
		debug_error("pFileName");
		return MM_ERROR_IMAGE_FILEOPEN;
	}

	infile = fopen(pFileName, "rb");
	if(infile == NULL) {
		debug_error("[infile] Exit on error");
		return MM_ERROR_IMAGE_FILEOPEN;
	}
	debug_log("infile");
	/* allocate and initialize JPEG decompression object   We set up the normal JPEG error routines, then override error_exit. */
	dinfo.err = jpeg_std_error(&jerr.pub);
	debug_log("jpeg_std_error");
	jerr.pub.error_exit = my_error_exit;
	debug_log("jerr.pub.error_exit ");

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.  We need to clean up the JPEG object, close the input file, and return.*/
		debug_log("setjmp");
		jpeg_destroy_decompress(&dinfo);
		fclose(infile);
		return MM_ERROR_IMAGE_FILEOPEN;
	}

	debug_log("if(setjmp)");
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&dinfo);
	debug_log("jpeg_create_decompress");

	/*specify data source (eg, a file) */
	jpeg_stdio_src(&dinfo, infile);
	debug_log("jpeg_stdio_src");

	/*read file parameters with jpeg_read_header() */
	jpeg_read_header(&dinfo, TRUE);
#if PARTIAL_DECODE
	dinfo.do_fancy_upsampling = FALSE;
	dinfo.do_block_smoothing = FALSE;
	dinfo.dither_mode = JDITHER_ORDERED;
#endif
	dinfo.dct_method = JDCT_FASTEST;

	/* set parameters for decompression */
	if (input_fmt == MM_UTIL_JPEG_FMT_RGB888) {
		dinfo.out_color_space=JCS_RGB;
		decoded_data->format = MM_UTIL_JPEG_FMT_RGB888;
		debug_log("cinfo.out_color_space=JCS_RGB");
	} else if(input_fmt == MM_UTIL_JPEG_FMT_YUV420 || input_fmt == MM_UTIL_JPEG_FMT_YUV422 || input_fmt == MM_UTIL_JPEG_FMT_UYVY) {
		dinfo.out_color_space=JCS_YCbCr;
		decoded_data->format = input_fmt;
		debug_log("cinfo.out_color_space=JCS_YCbCr");
	} else if(input_fmt == MM_UTIL_JPEG_FMT_GraySacle) {
		dinfo.out_color_space=JCS_GRAYSCALE;
		decoded_data->format = MM_UTIL_JPEG_FMT_GraySacle;
		debug_log("cinfo.out_color_space=JCS_GRAYSCALE");
	}

	/* Start decompressor*/
	jpeg_start_decompress(&dinfo);

	/* JSAMPLEs per row in output buffer */
	row_stride = dinfo.output_width * dinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*dinfo.mem->alloc_sarray) ((j_common_ptr) &dinfo, JPOOL_IMAGE, row_stride, 1);
	debug_log("buffer");
	decoded_data->width = dinfo.output_width;
	decoded_data->height= dinfo.output_height;
	if(input_fmt == MM_UTIL_JPEG_FMT_RGB888) {
		decoded_data->size= dinfo.output_height * row_stride;
	} else if(input_fmt == MM_UTIL_JPEG_FMT_YUV420) {
		decoded_data->size= dinfo.output_height * row_stride / 2;
	} else if(input_fmt == MM_UTIL_JPEG_FMT_YUV422 || input_fmt == MM_UTIL_JPEG_FMT_UYVY) {
		decoded_data->size= dinfo.output_height * dinfo.output_width * 2;
	} else if(input_fmt == MM_UTIL_JPEG_FMT_GraySacle) {
		decoded_data->size= dinfo.output_height * dinfo.output_width;
	} else{
		debug_error("We can't decode the IMAGE format");
		fclose(infile);
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	decoded_data->data= (void*) g_malloc0(decoded_data->size);
	decoded_data->format = input_fmt;

	if(decoded_data->data == NULL) {
		debug_error("decoded_data->data is NULL");
		fclose(infile);
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}
	debug_log("decoded_data->data");

	if(input_fmt == MM_UTIL_JPEG_FMT_YUV420 || input_fmt == MM_UTIL_JPEG_FMT_YUV422 || input_fmt == MM_UTIL_JPEG_FMT_UYVY) {
		image = decoded_data->data;
		u_image = image + (dinfo.output_width * dinfo.output_height);
		v_image = u_image + (dinfo.output_width*dinfo.output_height)/4;
		row= buffer[0];
		int i = 0;
		int y = 0;
		while (dinfo.output_scanline < dinfo.output_height) {
			jpeg_read_scanlines(&dinfo, buffer, 1);
			for (i = 0; i < row_stride; i += 3) {
				image[i/3]=row[i];
				if (i & 1) {
					u_image[(i/3)/2]=row[i+1];
					v_image[(i/3)/2]=row[i+2];
				}
			}
			image += row_stride/3;
			if (y++ & 1) {
				u_image += dinfo.output_width / 2;
				v_image += dinfo.output_width / 2;
			}
		}
	} else if(input_fmt == MM_UTIL_JPEG_FMT_RGB888 ||input_fmt == MM_UTIL_JPEG_FMT_GraySacle) {
		/* while (scan lines remain to be read) jpeg_read_scanlines(...); */
		while (dinfo.output_scanline < dinfo.output_height) {
			/* jpeg_read_scanlines expects an array of pointers to scanlines. Here the array is only one element long, but you could ask formore than one scanline at a time if that's more convenient. */
			jpeg_read_scanlines(&dinfo, buffer, 1);
			memcpy(decoded_data->data + state, buffer[0], row_stride);
			state += row_stride;
		}
		debug_log("while");
	}

	/* Finish decompression */
	jpeg_finish_decompress(&dinfo);

	/* Release JPEG decompression object */
	jpeg_destroy_decompress(&dinfo);

	fclose(infile);
	debug_log("fclose");

	return iErrorCode;
}

static int
mm_image_decode_from_jpeg_memory_with_libjpeg(mm_util_jpeg_yuv_data * decoded_data, void *src, int size, mm_util_jpeg_yuv_format input_fmt)
{
	int iErrorCode	= MM_ERROR_NONE;
	struct jpeg_decompress_struct dinfo;
	struct my_error_mgr_s jerr;
	JSAMPARRAY buffer; /* Output row buffer */
	int row_stride = 0, state = 0; /* physical row width in output buffer */
	JSAMPROW image, u_image, v_image;
	JSAMPROW row; /* point to buffer[0] */

	debug_log("Enter");

	if(src == NULL) {
		debug_error("[infile] Exit on error");
		return MM_ERROR_IMAGE_FILEOPEN;
	}
	debug_log("infile");

	/* allocate and initialize JPEG decompression object   We set up the normal JPEG error routines, then override error_exit. */
	dinfo.err = jpeg_std_error(&jerr.pub);
	debug_log("jpeg_std_error ");
	jerr.pub.error_exit = my_error_exit;
	debug_log("jerr.pub.error_exit ");

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.  We need to clean up the JPEG object, close the input file, and return.*/
		debug_log("setjmp");
		jpeg_destroy_decompress(&dinfo);
		return MM_ERROR_IMAGE_FILEOPEN;
	}

	debug_log("if(setjmp)");
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&dinfo);
	debug_log("jpeg_create_decompress");

	/*specify data source (eg, a file) */
	jpeg_mem_src(&dinfo, src, size);
	debug_log("jpeg_stdio_src");

	/*read file parameters with jpeg_read_header() */
	jpeg_read_header(&dinfo, TRUE);
	debug_log("jpeg_read_header");
	dinfo.dct_method = JDCT_FASTEST;

	/* set parameters for decompression */
	if (input_fmt == MM_UTIL_JPEG_FMT_RGB888) {
		dinfo.out_color_space=JCS_RGB;
		decoded_data->format = MM_UTIL_JPEG_FMT_RGB888;
		debug_log("cinfo.out_color_space=JCS_RGB");
	} else if(input_fmt == MM_UTIL_JPEG_FMT_YUV420 || input_fmt == MM_UTIL_JPEG_FMT_YUV422 || input_fmt == MM_UTIL_JPEG_FMT_UYVY) {
		dinfo.out_color_space=JCS_YCbCr;
		decoded_data->format = input_fmt;
		debug_log("cinfo.out_color_space=JCS_YCbCr");
	} else if(input_fmt == MM_UTIL_JPEG_FMT_GraySacle) {
		dinfo.out_color_space=JCS_GRAYSCALE;
		decoded_data->format = MM_UTIL_JPEG_FMT_GraySacle;
		debug_log("cinfo.out_color_space=JCS_GRAYSCALE");
	}

	/* Start decompressor*/
	jpeg_start_decompress(&dinfo);
	debug_log("jpeg_start_decompress");

	/* JSAMPLEs per row in output buffer */
	row_stride = dinfo.output_width * dinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*dinfo.mem->alloc_sarray) ((j_common_ptr) &dinfo, JPOOL_IMAGE, row_stride, 1);
	debug_log("buffer");
	decoded_data->width = dinfo.output_width;
	decoded_data->height= dinfo.output_height;
	if(input_fmt == MM_UTIL_JPEG_FMT_RGB888) {
		decoded_data->size= dinfo.output_height * row_stride;
	} else if(input_fmt == MM_UTIL_JPEG_FMT_YUV420) {
		decoded_data->size= dinfo.output_height * row_stride / 2;
	} else if(input_fmt == MM_UTIL_JPEG_FMT_YUV422 || input_fmt == MM_UTIL_JPEG_FMT_UYVY) {
		decoded_data->size= dinfo.output_height * dinfo.output_width * 2;
	} else if(input_fmt == MM_UTIL_JPEG_FMT_GraySacle) {
		decoded_data->size= dinfo.output_height * dinfo.output_width;
	} else{
		debug_error("We can't decode the IMAGE format");
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	}

	decoded_data->data= (void*) g_malloc0(decoded_data->size);
	decoded_data->format = input_fmt;

	if(decoded_data->data == NULL) {
		debug_error("decoded_data->data is NULL");
		return MM_ERROR_IMAGE_NO_FREE_SPACE;
	}
	debug_log("decoded_data->data");

	/* while (scan lines remain to be read) jpeg_read_scanlines(...); */
	if(input_fmt == MM_UTIL_JPEG_FMT_YUV420 || input_fmt == MM_UTIL_JPEG_FMT_YUV422 || input_fmt == MM_UTIL_JPEG_FMT_UYVY) {
		image = decoded_data->data;
		u_image = image + (dinfo.output_width * dinfo.output_height);
		v_image = u_image + (dinfo.output_width*dinfo.output_height)/4;
		row= buffer[0];
		int i = 0;
		int y = 0;
		while (dinfo.output_scanline < dinfo.output_height) {
			jpeg_read_scanlines(&dinfo, buffer, 1);
			for (i = 0; i < row_stride; i += 3) {
				image[i/3]=row[i];
				if (i & 1) {
					u_image[(i/3)/2]=row[i+1];
					v_image[(i/3)/2]=row[i+2];
				}
			}
			image += row_stride/3;
			if (y++ & 1) {
				u_image += dinfo.output_width / 2;
				v_image += dinfo.output_width / 2;
			}
		}
	} else if(input_fmt == MM_UTIL_JPEG_FMT_RGB888 ||input_fmt == MM_UTIL_JPEG_FMT_GraySacle) {
		while (dinfo.output_scanline < dinfo.output_height) {
			/* jpeg_read_scanlines expects an array of pointers to scanlines. Here the array is only one element long, but you could ask formore than one scanline at a time if that's more convenient. */
			jpeg_read_scanlines(&dinfo, buffer, 1);

			memcpy(decoded_data->data + state, buffer[0], row_stride);
			state += row_stride;
		}
		debug_log("while");

	}

	/* Finish decompression */
	jpeg_finish_decompress(&dinfo);
	debug_log("jpeg_finish_decompress");

	/* Release JPEG decompression object */
	jpeg_destroy_decompress(&dinfo);
	debug_log("jpeg_destroy_decompress");

	debug_log("fclose");

	return iErrorCode;
}

EXPORT_API int
mm_util_jpeg_encode_to_file(char *filename, void* src, int width, int height, mm_util_jpeg_yuv_format fmt, int quality)
{
	int ret = MM_ERROR_NONE;
	int use_hw_codec = 0;
	char *env_value = NULL;

	if( !filename || !src) {
		debug_error("#ERROR# filename || src buffer is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (width < 0) || (height < 0)) {
		debug_error("#ERROR# src_width || src_height value ");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (fmt < MM_UTIL_JPEG_FMT_YUV420) || (fmt > MM_UTIL_JPEG_FMT_NV61) ) {
		debug_error("#ERROR# fmt value");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (quality < 0 ) || (quality>100) ) {
		debug_error("#ERROR# quality vaule");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	/* check environment value */
	env_value = getenv(ENV_NAME_USE_HW_CODEC);
	if (env_value) {
		use_hw_codec = atoi(env_value);
		debug_log("%s - value str:%s -> int:%d",
		                         ENV_NAME_USE_HW_CODEC, env_value, use_hw_codec);
	}

	/* check whether support HW codec */
	if (use_hw_codec && fmt != MM_UTIL_JPEG_FMT_RGB888 && width <= ENC_MAX_LEN && height <= ENC_MAX_LEN) {
		int size = 0;
		void *mem = NULL;
		void *dl_handle = NULL;
		EncodeJPEGFunc encode_jpeg_func = NULL;

		/* dlopen library */
		dl_handle = dlopen(LIB_PATH_HW_CODEC_LIBRARY, RTLD_LAZY);
		if (!dl_handle) {
			debug_warning("H/W JPEG not supported");
			goto JPEG_SW_ENCODE;
		}

		/* find function symbol */
		encode_jpeg_func = (EncodeJPEGFunc)dlsym(dl_handle, ENCODE_JPEG_HW_FUNC_NAME);
		if (!encode_jpeg_func) {
			debug_warning("find func[%s] failed",
			                             ENCODE_JPEG_HW_FUNC_NAME);
			/* close dl_handle */
			dlclose(dl_handle);
			dl_handle = NULL;
			goto JPEG_SW_ENCODE;
		}

		/* Do H/W jpeg encoding */
		ret = encode_jpeg_func(src, width, height, fmt, quality, (unsigned char **)&mem, &size);

		/* close dl_handle */
		dlclose(dl_handle);
		dl_handle = NULL;

		if (ret == MM_ERROR_NONE) {
			debug_log("HW ENCODE DONE.. Write file");
			if (mem && size > 0) {
				if (!_write_file(filename, mem, size)) {
					debug_error("failed Write file");
					ret = MM_ERROR_IMAGE_INTERNAL;
				}

				free(mem);
				mem = NULL;
				size = 0;
				sync();

				return ret;
			} else {
				debug_warning("invalid data %p, size %d", mem, size);
			}
		} else {
			debug_warning("HW encode failed %x", ret);
		}
	}

JPEG_SW_ENCODE:
	#if LIBJPEG_TURBO
	debug_log("#START# LIBJPEG_TURBO");
	ret=mm_image_encode_to_jpeg_file_with_libjpeg_turbo(filename, src, width, height, fmt, quality);
	debug_log("#End# libjpeg, Success!! ret: %d", ret);

	#else
	debug_log("#START# LIBJPEG");
	if(fmt == MM_UTIL_JPEG_FMT_NV12) {
		int dst_size = 0;
		ret = mm_util_get_image_size(MM_UTIL_IMG_FMT_NV12, width, height, &dst_size);
		unsigned char *dst = NULL;
		dst = malloc(dst_size);
		if(dst) {
			ret = mm_util_convert_colorspace(src, width, height,MM_UTIL_IMG_FMT_NV12, dst, MM_UTIL_IMG_FMT_YUV420);
			ret =mm_image_encode_to_jpeg_file_with_libjpeg(filename, dst, width, height, MM_UTIL_JPEG_FMT_YUV420, quality);
			free(dst); dst=NULL;
		} else {
			return MM_ERROR_IMAGE_NO_FREE_SPACE;
		}
	} else if(fmt == MM_UTIL_JPEG_FMT_NV21) {
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	} else {
		ret = mm_image_encode_to_jpeg_file_with_libjpeg(filename, src, width, height, fmt, quality);
	}
	debug_log("#END# libjpeg, Success!! ret: %d", ret);
	#endif
	return ret;
}

EXPORT_API int
mm_util_jpeg_encode_to_memory(void **mem, int *size, void* src, int width, int height, mm_util_jpeg_yuv_format fmt, int quality)
{
	int ret = MM_ERROR_NONE;
	int use_hw_codec = 0;
	char *env_value = NULL;

	if( !mem || !size || !src) {
		debug_error("#ERROR# filename ||size ||  src buffer is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (width < 0) || (height < 0)) {
		debug_error("#ERROR# src_width || src_height value ");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (fmt < MM_UTIL_JPEG_FMT_YUV420) || (fmt > MM_UTIL_JPEG_FMT_NV61) ) {
		debug_error("#ERROR# fmt value");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if(  (quality < 0 ) || (quality>100) ) {
		debug_error("#ERROR# quality vaule");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	/* check environment value */
	env_value = getenv(ENV_NAME_USE_HW_CODEC);
	if (env_value) {
		use_hw_codec = atoi(env_value);
		debug_log("%s - value str:%s -> int:%d",
		                         ENV_NAME_USE_HW_CODEC, env_value, use_hw_codec);
	}

	/* check whether support HW codec */
	if (use_hw_codec && fmt != MM_UTIL_JPEG_FMT_RGB888 && width <= ENC_MAX_LEN && height <= ENC_MAX_LEN) {
		void *dl_handle = NULL;
		EncodeJPEGFunc encode_jpeg_func = NULL;

		/* dlopen library */
		dl_handle = dlopen(LIB_PATH_HW_CODEC_LIBRARY, RTLD_LAZY);
		if (!dl_handle) {
			debug_warning("H/W JPEG not supported");
			goto JPEG_SW_ENCODE;
		}

		/* find function symbol */
		encode_jpeg_func = (EncodeJPEGFunc)dlsym(dl_handle, ENCODE_JPEG_HW_FUNC_NAME);
		if (!encode_jpeg_func) {
			debug_warning("find func[%s] failed",
			                             ENCODE_JPEG_HW_FUNC_NAME);
			/* close dl_handle */
			dlclose(dl_handle);
			dl_handle = NULL;
			goto JPEG_SW_ENCODE;
		}

		/* Do H/W jpeg encoding */
		ret = encode_jpeg_func(src, width, height, fmt, quality, (unsigned char **)mem, size);

		/* close dl_handle */
		dlclose(dl_handle);
		dl_handle = NULL;

		if (ret == MM_ERROR_NONE) {
			debug_log("HW ENCODE DONE");
			return ret;
		} else {
			debug_warning("HW encode failed %x", ret);
		}
	}

JPEG_SW_ENCODE:
	#if LIBJPEG_TURBO
	debug_log("#START# libjpeg-turbo");
	ret=mm_image_encode_to_jpeg_memory_with_libjpeg_turbo(mem, size, src, width, height, fmt, quality);
	debug_log("#END# libjpeg-turbo, Success!! ret: %d", ret);
	#else /* LIBJPEG_TURBO */
	debug_log("#START# libjpeg");
	if(fmt == MM_UTIL_JPEG_FMT_NV12) {
		int dst_size = 0;
		ret = mm_util_get_image_size(MM_UTIL_IMG_FMT_NV12, width, height, &dst_size);
		unsigned char *dst = NULL;
		dst = malloc(dst_size);
		if(dst) {
			ret = mm_util_convert_colorspace(src, width, height,MM_UTIL_IMG_FMT_NV12, dst, MM_UTIL_IMG_FMT_YUV420);
			ret = mm_image_encode_to_jpeg_memory_with_libjpeg(mem, size, dst, width, height, MM_UTIL_JPEG_FMT_YUV420, quality);
			free(dst); dst=NULL;
		} else {
			return MM_ERROR_IMAGE_NO_FREE_SPACE;
		}
	} else if(fmt == MM_UTIL_JPEG_FMT_NV21) {
		return MM_ERROR_IMAGE_NOT_SUPPORT_FORMAT;
	} else {
		ret = mm_image_encode_to_jpeg_memory_with_libjpeg(mem, size, src, width, height, fmt, quality);
	}
	#endif /* LIBJPEG_TURBO */
	debug_log("#END# libjpeg, Success!! ret: %d", ret);
	return ret;
}

EXPORT_API int
mm_util_decode_from_jpeg_file(mm_util_jpeg_yuv_data *decoded, char *filename, mm_util_jpeg_yuv_format fmt)
{
	int ret = MM_ERROR_NONE;
	int use_hw_codec = 0;
	char *env_value = NULL;

	if( !decoded || !filename) {
		debug_error("#ERROR# decoded || filename buffer is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (fmt < MM_UTIL_JPEG_FMT_YUV420) || (fmt > MM_UTIL_JPEG_FMT_NV61) ) {
		debug_error("#ERROR# fmt value");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	/* check environment value */
	env_value = getenv(ENV_NAME_USE_HW_CODEC);
	if (env_value) {
		use_hw_codec = atoi(env_value);
		debug_log("%s - value str:%s -> int:%d",
			ENV_NAME_USE_HW_CODEC, env_value, use_hw_codec);
	}
	/* check whether support HW codec */
	if (use_hw_codec && fmt != MM_UTIL_JPEG_FMT_RGB888) {
		void *src = NULL;
		int src_size = 0;
		void *dl_handle = NULL;
		DecodeJPEGFunc decode_jpeg_func = NULL;

		debug_log("START H/W JPEG DECODE");

		/* dlopen library */
		dl_handle = dlopen(LIB_PATH_HW_CODEC_LIBRARY, RTLD_LAZY);
		if (!dl_handle) {
			debug_warning("H/W JPEG not supported");
			goto JPEG_SW_DECODE;
		}

		/* find function symbol */
		decode_jpeg_func = (DecodeJPEGFunc)dlsym(dl_handle, DECODE_JPEG_HW_FUNC_NAME);
		if (!decode_jpeg_func) {
			debug_warning("find func[%s] failed",
							DECODE_JPEG_HW_FUNC_NAME);
			/* close dl_handle */
			dlclose(dl_handle);
			dl_handle = NULL;
			goto JPEG_SW_DECODE;
		}

		if (!_read_file(filename, &src, &src_size)) {
			debug_warning("read file [%s] failed",
			DECODE_JPEG_HW_FUNC_NAME);
			/* close dl_handle */
			dlclose(dl_handle);
			dl_handle = NULL;
			return MM_ERROR_FILE_CLASS;
		}

		/* Do H/W jpeg decoding */
		ret = decode_jpeg_func(src, src_size, fmt, (unsigned char **)&decoded->data, &decoded->width, &decoded->height, &decoded->size);

		/* release src buffer */
		if (src) {
			free(src);
			src_size = 0;
		}

		/* close dl_handle */
		dlclose(dl_handle);
		dl_handle = NULL;

		/* check decoding result */
		if (ret == MM_ERROR_NONE) {
			decoded->format = fmt;
			debug_log("HW decode DONE");
			return ret;
		} else {
			debug_warning("HW decode failed %x",ret);
		}
	}
JPEG_SW_DECODE:
	#if LIBJPEG_TURBO
	debug_log("#START# LIBJPEG_TURBO");
	ret = mm_image_decode_from_jpeg_file_with_libjpeg_turbo(decoded, filename, fmt);
	debug_log("decoded->data: %p\t width: %d\t height: %d\t size: %d\n", decoded->data, decoded->width, decoded->height, decoded->size);
	debug_log("#End# LIBJPEG_TURBO, Success!! ret: %d", ret);
	#else
	debug_log("#START# libjpeg");
	ret = mm_image_decode_from_jpeg_file_with_libjpeg(decoded, filename, fmt);
	debug_log("decoded->data: %p\t width: %d\t height:%d\t size: %d\n", decoded->data, decoded->width, decoded->height, decoded->size);
	debug_log("#End# libjpeg, Success!! ret: %d", ret);
	#endif
	return ret;
}

EXPORT_API int
mm_util_decode_from_jpeg_memory(mm_util_jpeg_yuv_data* decoded, void* src, int size, mm_util_jpeg_yuv_format fmt)
{
	int ret = MM_ERROR_NONE;
	int use_hw_codec = 0;
	char *env_value = NULL;

	if( !decoded || !src) {
		debug_error("#ERROR# decoded || src buffer is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if(size < 0) {
		debug_error("#ERROR# size");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if( (fmt < MM_UTIL_JPEG_FMT_YUV420) || (fmt > MM_UTIL_JPEG_FMT_NV61) ) {
		debug_error("#ERROR# fmt value");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}
	/* check environment value */
	env_value = getenv(ENV_NAME_USE_HW_CODEC);
	if (env_value) {
		use_hw_codec = atoi(env_value);
		debug_log("%s - value str:%s -> int:%d",
							ENV_NAME_USE_HW_CODEC, env_value, use_hw_codec);
	}
	/* check whether support HW codec */
	if (use_hw_codec && fmt != MM_UTIL_JPEG_FMT_RGB888) {
		void *dl_handle = NULL;
		DecodeJPEGFunc decode_jpeg_func = NULL;
		debug_log("START H/W JPEG DECODE");

		/* dlopen library */
		dl_handle = dlopen(LIB_PATH_HW_CODEC_LIBRARY, RTLD_LAZY);
		if (!dl_handle) {
			debug_warning("H/W JPEG not supported");
			goto JPEG_SW_DECODE;
		}
		/* find function symbol */
		decode_jpeg_func = (DecodeJPEGFunc)dlsym(dl_handle, DECODE_JPEG_HW_FUNC_NAME);
		if (!decode_jpeg_func) {
			debug_warning("find func[%s] failed",
						               DECODE_JPEG_HW_FUNC_NAME);
			/* close dl_handle */
			dlclose(dl_handle);
			dl_handle = NULL;
			goto JPEG_SW_DECODE;
		}
		/* Do H/W jpeg encoding */

		ret = decode_jpeg_func(src, size, fmt, (unsigned char **)&decoded->data, &decoded->width, &decoded->height, &decoded->size);
		/* close dl_handle */
		dlclose(dl_handle);
		dl_handle = NULL;

		/* check decoding result */
		if (ret == MM_ERROR_NONE) {
			decoded->format = fmt;
			debug_log("HW DECODE DONE");
			return ret;
		} else {
			debug_warning("HW decode failed %x", ret);
		}
	}

JPEG_SW_DECODE:
	#if LIBJPEG_TURBO
	debug_log("#START# libjpeg");
	ret = mm_image_decode_from_jpeg_memory_with_libjpeg_turbo(decoded, src, size, fmt);

	debug_log("decoded->data: %p\t width: %d\t height: %d\t size: %d\n", decoded->data, decoded->width, decoded->height, decoded->size);
	debug_log("#END# libjpeg, Success!! ret: %d", ret);

	#else
	debug_log("#START# libjpeg");
	ret = mm_image_decode_from_jpeg_memory_with_libjpeg(decoded, src, size, fmt);

	debug_log("decoded->data: %p\t width: %d\t height: %d\t size: %d\n", decoded->data, decoded->width, decoded->height, decoded->size);
	debug_log("#END# libjpeg, Success!! ret: %d", ret);
	#endif
	return ret;
}


static int _read_file(char *file_name, void **data, int *data_size)
{
	FILE *fp = NULL;
	int file_size = 0;

	if (!file_name || !data || !data_size) {
		debug_error("NULL pointer");
		return FALSE;
	}

	debug_log("Try to open %s to read", file_name);

	fp = fopen(file_name, "r");
	if (fp == NULL) {
		debug_error("file open failed errno %d", errno);
		return FALSE;
	}

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);
	*data = (void *)malloc(file_size);
	if (*data == NULL) {
		debug_error("malloc failed errno %d", errno);
	} else {
		fread(*data, 1, file_size, fp);
	}

	fclose(fp);
	fp = NULL;

	if (*data) {
		*data_size = file_size;
		return TRUE;
	} else {
		*data_size = 0;
		return FALSE;
	}
}


static int _write_file(char *file_name, void *data, int data_size)
{
	FILE *fp = NULL;

	if (!file_name || !data || data_size <= 0) {
		debug_error("invalid data %s %p size:%d", file_name, data, data_size);
		return FALSE;
	}

	debug_log("Try to open %s to write", file_name);

	fp = fopen(file_name, "w");
	if (fp) {
		fwrite(data, 1, data_size, fp);
		fclose(fp);
		fp = NULL;
	} else {
		debug_error("file open [%s] failed errno %d", file_name, errno);
		return FALSE;
	}

	debug_log("file [%s] write DONE", file_name);

	return TRUE;
}


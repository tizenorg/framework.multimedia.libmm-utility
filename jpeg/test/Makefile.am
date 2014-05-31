bin_PROGRAMS = mm_util_jpeg_testsuite

mm_util_jpeg_testsuite_SOURCES = mm_util_jpeg_testsuite.c

mm_util_jpeg_testsuite_CFLAGS = -I$(srcdir)/../include
mm_util_jpeg_testsuite_CFLAGS += $(MMTA_CFLAGS) -DGST_EXT_TIME_ANALYSIS \
				 $(MMCOMMON_CFLAGS) \
				 $(MMLOG_CFLAGS) -DMMF_LOG_OWNER=0x0100 -DMMF_DEBUG_PREFIX=\"MMF-IMAGE\"


############################################

mm_util_jpeg_testsuite_DEPENDENCIES = $(srcdir)/../libmmutil_jpeg.la 
						
mm_util_jpeg_testsuite_LDADD = $(srcdir)/../libmmutil_jpeg.la 
mm_util_jpeg_testsuite_LDADD += $(MMTA_LIBS) \
				$(MMCOMMON_LIBS)


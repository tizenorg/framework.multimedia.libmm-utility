bin_PROGRAMS = mm_util_imgp_testsuite

mm_util_imgp_testsuite_SOURCES = mm_util_imgp_testsuite.c

mm_util_imgp_testsuite_CFLAGS = -I$(srcdir)/../include \
                                  $(MMTA_CFLAGS) -DGST_EXT_TIME_ANALYSIS \
				  $(MMCOMMON_CFLAGS) \
				  $(MMCAMCORDER_CFLAGS) \
				  $(MMLOG_CFLAGS) -DMMF_LOG_OWNER=0x0100 -DMMF_DEBUG_PREFIX=\"MMF-IMAGE\" -D_INTERNAL_SESSION_MANAGER_

############################################

mm_util_imgp_testsuite_DEPENDENCIES = $(srcdir)/../libmmutil_imgp.la 
						
mm_util_imgp_testsuite_LDADD = $(srcdir)/../libmmutil_imgp.la \
 			       $(MMTA_LIBS) \
 			       $(MMCOMMON_LIBS) \
			       $(MMCAMCORDER_LIBS) \
			       $(MMLOG_LIBS)


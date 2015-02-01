bin_PROGRAMS = mm_util_imgp_testsuite

mm_util_imgp_testsuite_SOURCES = mm_util_imgp_testsuite.c

mm_util_imgp_testsuite_CFLAGS = -I$(srcdir)/../include \
                                -I$(includedir)/drm \
				  $(MMCOMMON_CFLAGS) \
				  $(MMCAMCORDER_CFLAGS) \
				  $(DRM_CFLAGS) \
				  $(DRM_DEVEL_CFLAGS) \
				  $(MEDIA_CFLAGS) \
				  $(XFIXES_CFLAGS) \
				  $(DRI2PROTO_CFLAGS) \
				  $(DRI2_CFLAGS) \
				  $(TBM_CFLAGS) \
				  $(MMLOG_CFLAGS) -DMMF_LOG_OWNER=0x0100 -DMMF_DEBUG_PREFIX=\"MMF-IMAGE\" -D_INTERNAL_SESSION_MANAGER_

############################################

mm_util_imgp_testsuite_DEPENDENCIES = $(srcdir)/../libmmutil_imgp.la 
						
mm_util_imgp_testsuite_LDADD = $(srcdir)/../libmmutil_imgp.la \
 			       $(MMCOMMON_LIBS) \
			       $(MMCAMCORDER_LIBS) \
			       $(DRM_LIBS) \
			       $(DRM_DEVEL_LIBS) \
			       $(MEDIA_LIBS) \
			       $(XFIXES_LIBS) \
			       $(DRI2PROTO_LIBS) \
			       $(DRI2_LIBS) \
			       $(TBM_LIBS) \
			       $(MMLOG_LIBS)


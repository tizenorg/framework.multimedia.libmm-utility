ACLOCAL_AMFLAGS='-I m4'

lib_LTLIBRARIES = libmmutil_imgp.la

installmmutil_imgpdir = $(includedir)/mmf
installmmutil_imgp_HEADERS = include/mm_util_imgp.h
noinst_HEADERS = include/mm_util_imgp_internal.h

libmmutil_imgp_la_SOURCES = mm_util_imgp.c
	
libmmutil_imgp_la_CFLAGS = -I$(srcdir)/include \
			   -I$(includedir)/drm \
			   $(MMCOMMON_CFLAGS) \
                           $(GLIB_CFLAGS) \
                           $(GMODULE_CFLAGS) \
                           $(DRM_CFLAGS) \
                           $(DRM_DEVEL_CFLAGS) \
                           $(MEDIA_CFLAGS) \
                           $(XFIXES_CFLAGS) \
                           $(DRI2PROTO_CFLAGS) \
                           $(DRI2_CFLAGS) \
                           $(TBM_CFLAGS) \
                           $(MMLOG_CFLAGS)
			
libmmutil_imgp_la_LIBADD = $(MMCOMMON_LIBS) \
                           $(GLIB_LIBS) \
                           $(GMODULE_LIBS) \
                           $(DRM_LIBS) \
                           $(DRM_DEVEL_LIBS) \
                           $(MEDIA_LIBS) \
                           $(XFIXES_LIBS) \
                           $(DRI2PROTO_LIBS) \
                           $(DRI2_LIBS) \
                           $(TBM_LIBS) \
                           $(MMLOG_LIBS)

libmmutil_imgp_la_CFLAGS += -lcapi-media-tool -DMMF_LOG_OWNER=0x0100 -DMMF_DEBUG_PREFIX=\"MMF-IMAGE\"

pcfiles = mmutil-imgp.pc
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)
EXTRA_DIST = $(pcfiles)
SUBDIRS = . test

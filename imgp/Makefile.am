ACLOCAL_AMFLAGS='-I m4'

lib_LTLIBRARIES = libmmutil_imgp.la

installmmutil_imgpdir = $(includedir)/mmf
installmmutil_imgp_HEADERS = include/mm_util_imgp.h
noinst_HEADERS = include/mm_util_imgp_internal.h

libmmutil_imgp_la_SOURCES = mm_util_imgp.c
	
libmmutil_imgp_la_CFLAGS = -I$(srcdir)/include \
			   $(MMCOMMON_CFLAGS) \
                           $(MMTA_CFLAGS) \
                           $(GLIB_CFLAGS) \
                           $(GMODULE_CFLAGS) \
                           $(MMLOG_CFLAGS) -DMMF_LOG_OWNER=0x0100 -DMMF_DEBUG_PREFIX=\"MMF-IMAGE\"
			
libmmutil_imgp_la_LIBADD = $(MMCOMMON_LIBS) \
			   $(MMTA_LIBS) \
                           $(GLIB_LIBS) \
                           $(GMODULE_LIBS) \
                           $(MMLOG_LIBS)

libmmutil_imgp_la_CFLAGS += $(MMLOG_CFLAGS) -DMMF_LOG_OWNER=0x0100 -DMMF_DEBUG_PREFIX=\"MMF-IMAGE\"

pcfiles = mmutil-imgp.pc
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)
EXTRA_DIST = $(pcfiles)
SUBDIRS = . test

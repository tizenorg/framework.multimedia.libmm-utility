bin_PROGRAMS = mm_utility_testsuite

mm_utility_testsuite_SOURCES = mm_utility_testsuite.c	
			     
mm_utility_testsuite_CFLAGS = -I$(srcdir)/../include \
                                $(MMCOMMON_CFLAGS)

						
mm_utility_testsuite_LDADD = $(MMCOMMON_LIBS)


mm_utility_testsuite_CFLAGS += $(SAVSIMGD_CFLAGS)
mm_utility_testsuite_LDADD += $(SAVSIMGD_LIBS)

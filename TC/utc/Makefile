ifeq ($(ARCH),target)
	PKG_CONFIG_PATH=/usr/lib/pkgconfig
	export PKG_CONFIG_PATH
	CC=arm-linux-gcc -Wall
	CXX=arm-linux-g++ -Wall
else
	PKG_CONFIG_PATH=/usr/lib/pkgconfig
	export PKG_CONFIG_PATH
	CC=gcc -Wall
    CXX=gcc -Wall
endif

TS1=utc_mm_util_convert_colorspace_func
TS2=utc_mm_util_decode_from_jpeg_file_func
TS3=utc_mm_util_decode_from_jpeg_memory_func
TS4=utc_mm_util_get_image_size_func
TS5=utc_mm_util_jpeg_encode_to_file_func
TS6=utc_mm_util_jpeg_encode_to_memory_func
TS7=utc_mm_util_resize_image_func
TS8=utc_mm_util_rotate_image_func

LIBS = `pkg-config --libs mmutil-imgp mmutil-jpeg glib-2.0`  
LIBS +=-L/usr/lib/:/usr/lib/pkgconfig
LIBS +=$(TET_ROOT)/lib/tet3/tcm_s.o
LIBS +=-L$(TET_ROOT)/lib/tet3 -ltcm_s
LIBS +=-L$(TET_ROOT)/lib/tet3/ -lapi_s

INCS = -I. `pkg-config --cflags mmutil-imgp mmutil-jpeg glib-2.0`
INCS += -I$(TET_ROOT)/inc/tet3
INCS += -I/usr/include/mmf

CFLAGS = $(INCS) 
CC += $(CFLAGS)
LDFLAGS = $(LIBS)

all : $(TS1) $(TS2) $(TS3) $(TS4) $(TS5) $(TS6) $(TS7) $(TS8)

$(TS1): $(TS1).c
	$(CC) -o $(TS1) $(TS1).c $(LDFLAGS)

$(TS2): $(TS2).c
	$(CC) -o $(TS2) $(TS2).c $(LDFLAGS)
	
$(TS3): $(TS3).c
	$(CC) -o $(TS3) $(TS3).c $(LDFLAGS)

$(TS4): $(TS4).c
	$(CC) -o $(TS4) $(TS4).c $(LDFLAGS)

$(TS5): $(TS5).c
	$(CC) -o $(TS5) $(TS5).c $(LDFLAGS)
	
$(TS6): $(TS6).c
	$(CC) -o $(TS6) $(TS6).c $(LDFLAGS)

$(TS7): $(TS7).c
	$(CC) -o $(TS7) $(TS7).c $(LDFLAGS)

$(TS8): $(TS8).c
	$(CC) -o $(TS8) $(TS8).c $(LDFLAGS)

clean:
	rm -rf *~ *.o $(TS1) $(TS2) $(TS3) $(TS4) $(TS5) $(TS6) $(TS7) $(TS8)
	



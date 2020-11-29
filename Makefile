ifdef PROCESSOR_ARCHITECTURE
  OS=NT
else
  OS=unix
endif

#This would be for the MinGW running on the NT kernel 
#No need for the wine emulation.
ifeq ($(OS),NT)

# Windows MinGW using Cgywin.
CC			= g++.exe
LD	 		= ld.exe

#WIN_INCLUDES		+= -I"/usr/i686-pc-mingw32/sys-root/mingw/include"
#WIN_LIBPATH		+= -L"/usr/i686-pc-mingw32/sys-root/mingw/lib"\
#			   -L"/usr/i686-pc-mingw32/sys-root/mingw/bin/libstdc++-6.dll"
WIN_LIBS		+= -lgcc -lodbc32 -lwsock32 -lwinspool\
			-lwinmm -lshell32 -lcomctl32 -lctl3d32\
			-lodbc32 -ladvapi32 -lodbc32 -lopengl32\
			-lglu32 -lole32 -loleaut32 -luuid\
			-lnetapi32 -lws2_32 

WIN_LDFLAGS 		+= -s 
WIN_CFLAGS		+= -static -mwindows -O2 -Wall\
			-g -DWINVER=0x0501\
			-D__WIN95__ -D__GNUWIN32__ -DSTRICT\
			-DHAVE_W32API_H -D__WXMSW__ -D__WINDOWS__\
			-DUNICODE -D_UNICODE\

LDFLAGS			= $(WIN_LDFLAGS)
CFLAGS			= $(WIN_CFLAGS)
# LIBPATH			= $(WIN_LIBPATH)
LIBS			= $(WIN_LIBS)

#This is for the BT5R3 MinGW release.
else

ADDR2LINE		= /usr/bin/i586-mingw32msvc-addr2line
AR 			= /usr/bin/i586-mingw32msvc-ar
AS			= /usr/bin/i586-mingw32msvc-as
CC			= /usr/bin/i586-mingw32msvc-cc
CCXXFILT		= /usr/bin/i586-mingw32msvc-c++filt
CPP			= /usr/bin/i586-mingw32msvc-cpp
DLLTOOL			= /usr/bin/i586-mingw32msvc-dlltool
DLLWRAP	 		= /usr/bin/i586-mingw32msvc-dllwrap
GXX			= /usr/bin/i586-mingw32msvc-g++
GCC			= /usr/bin/i586-mingw32msvc-gcc
GCC442			= /usr/bin/i586-mingw32msvc-gcc-4.4.2
GCCDEBUG		= /usr/bin/i586-mingw32msvc-gccbug
GCOV			= /usr/bin/i586-mingw32msvc-gcov
GFORTRAN		= /usr/bin/i586-mingw32msvc-gfortran
GPROF			= /usr/bin/i586-mingw32msvc-gprof
LD			= /usr/bin/i586-mingw32msvc-ld
NM			= /usr/bin/i586-mingw32msvc-nm
OBJCOPY			= /usr/bin/i586-mingw32msvc-objcopy
OBJDUMP			= /usr/bin/i586-mingw32msvc-objdump
RANLIB			= /usr/bin/i586-mingw32msvc-ranlib
READELF			= /usr/bin/i586-mingw32msvc-readelf
SIZE			= /usr/bin/i586-mingw32msvc-size
STRINGS			= /usr/bin/i586-mingw32msvc-strings
STRIP			= /usr/bin/i586-mingw32msvc-strip
WINDMC			= /usr/bin/i586-mingw32msvc-windmc
WINDRES			= /usr/bin/i586-mingw32msvc-windres

#Mingw for BT5R3

MINGW_INCLUDES		+= -I"/usr/i586-mingw32msvc/include"
MINGW_LIBPATH		+= -L"/usr/i586-mingw32msvc/lib"
MINGW_LIBS		+= -lgcc -lodbc32 -lwsock32 -lwinspool -lwinmm -lshell32 -lcomctl32\
			-lctl3d32 -lodbc32 -ladvapi32 -lodbc32 -lopengl32 -lglu32 -lole32 -loleaut32\
			-luuid -lnetapi32 -lstdc++ -lws2_32

MINGW_LDFLAGS 		+= -s 
MINGW_CFLAGS		+= -O2 -Wall -g -I. $(MINGW_INCLUDES)\
			-D__MINGW32__ -DWINVER=0x0501 -D__WIN95__ -D__GNUWIN32__\
			-DSTRICT -DHAVE_W32API_H -D__WXMSW__\
			-D__WINDOWS__ -DUNICODE -D_UNICODE\

LDFLAGS			= $(MINGW_LDFLAGS)
CFLAGS			= $(MINGW_CFLAGS)
LIBPATH			= $(MINGW_LIBPATH)
LIBS			= $(MINGW_LIBS)

endif

#==========================================================================
#Set the values of the program and source here!
#==========================================================================

OBJ_DIR			= MinGW_obj

PROGRAM			= netview.exe

SOURCE			= netview.cpp

OBJECTS			= $(addprefix $(OBJ_DIR)/, $(notdir $(SOURCE:.cpp=.o)))

none: clean
	@echo "usage:"
	@echo "make all"


all: clean $(OBJ_DIR) $(PROGRAM)
	@echo "Started to create the $(PROGRAM)"

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -municode -o $@ $^  $(LIBPATH) $(LIBS)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c  $<
	@echo

clean:
	rm -f $(OBJECTS) $(PROGRAM)

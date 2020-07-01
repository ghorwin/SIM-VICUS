# -------------------------------------------------
# Project for zlib library
# -------------------------------------------------

# first we define what we are
TARGET = zlib
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

# finally we setup our custom library specfic things
# like version number etc., we also may reset all
unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_AT}
}

INCLUDEPATH = \
		../../src/contrib/minizip \
		../../src/

SOURCES += \
	../../src/adler32.c \
	../../src/compress.c \
	../../src/crc32.c \
	../../src/deflate.c \
	../../src/gzclose.c \
	../../src/gzlib.c \
	../../src/gzread.c \
	../../src/gzwrite.c \
	../../src/inflate.c \
	../../src/infback.c \
	../../src/inftrees.c \
	../../src/inffast.c \
	../../src/trees.c \
	../../src/uncompr.c \
	../../src/zutil.c \
	../../src/contrib/minizip/ioapi.c \
	../../src/contrib/minizip/minizip.c \
	../../src/contrib/minizip/mztools.c \
	../../src/contrib/minizip/unzip.c \
	../../src/contrib/minizip/zip.c \
	../../src/contrib/minizip/miniunz.c

HEADERS += \
	../../src/crc32.h \
	../../src/deflate.h \
	../../src/gzguts.h \
	../../src/inffast.h \
	../../src/inffixed.h \
	../../src/inflate.h \
	../../src/inftrees.h \
	../../src/trees.h \
	../../src/zlib.h \
	../../src/zutil.h \
	../../src/contrib/minizip/crypt.h \
	../../src/contrib/minizip/ioapi.h \
	../../src/contrib/minizip/iowin32.h \
	../../src/contrib/minizip/mztools.h \
	../../src/contrib/minizip/unzip.h \
	../../src/contrib/minizip/zip.h

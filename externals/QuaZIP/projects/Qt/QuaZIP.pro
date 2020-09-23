# ---------------------------
# Project for QuaZIP library
# ---------------------------

TARGET = QuaZIP
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )


QT += core

DEFINES += QUAZIP_BUILD

unix|mac {
	VER_MAJ = 0
	VER_MIN = 7
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

INCLUDEPATH += ../../src/zlib/ \
				../../src/

SOURCES += \
	../../src/zip.c \
	../../src/unzip.c \
	../../src/quazipnewinfo.cpp \
	../../src/quazipfileinfo.cpp \
	../../src/quazipfile.cpp \
	../../src/quazipdir.cpp \
	../../src/quazip.cpp \
	../../src/quaziodevice.cpp \
	../../src/quagzipfile.cpp \
	../../src/quacrc32.cpp \
	../../src/quaadler32.cpp \
	../../src/qioapi.cpp \
	../../src/JlCompress.cpp \
	../../src/zlib/zutil.c \
	../../src/zlib/uncompr.c \
	../../src/zlib/trees.c \
	../../src/zlib/mztools.c \
	../../src/zlib/iowin32.c \
	../../src/zlib/ioapi.c \
	../../src/zlib/inftrees.c \
	../../src/zlib/inflate.c \
	../../src/zlib/inffast.c \
	../../src/zlib/infback.c \
	../../src/zlib/gzio.c \
	../../src/zlib/example.c \
	../../src/zlib/deflate.c \
	../../src/zlib/crc32.c \
	../../src/zlib/compress.c \
	../../src/zlib/adler32.c

HEADERS += \
	../../src/zip.h \
	../../src/unzip.h \
	../../src/quazipnewinfo.h \
	../../src/quazip_global.h \
	../../src/quazipfileinfo.h \
	../../src/quazipfile.h \
	../../src/quazipdir.h \
	../../src/quazip.h \
	../../src/quaziodevice.h \
	../../src/quagzipfile.h \
	../../src/quacrc32.h \
	../../src/quachecksum32.h \
	../../src/quaadler32.h \
	../../src/JlCompress.h \
	../../src/ioapi.h \
	../../src/crypt.h \
	../../src/zlib/zutil.h \
	../../src/zlib/zlib.h \
	../../src/zlib/zconf.in.h \
	../../src/zlib/zconf.h \
	../../src/zlib/trees.h \
	../../src/zlib/mztools.h \
	../../src/zlib/iowin32.h \
	../../src/zlib/ioapi.h \
	../../src/zlib/inftrees.h \
	../../src/zlib/inflate.h \
	../../src/zlib/inffixed.h \
	../../src/zlib/inffast.h \
	../../src/zlib/deflate.h \
	../../src/zlib/crypt.h \
	../../src/zlib/crc32.h


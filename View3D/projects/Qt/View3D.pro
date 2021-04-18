# -----------------------
# Project for View3D App
# -----------------------

# first we define what we are
TARGET = View3D
TEMPLATE = app

QMAKE_CFLAGS += -std=c89

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

CONFIG += console
CONFIG -= app_bundle

SOURCES += \
    ../../src/ctrans.c \
    ../../src/getdat.c \
    ../../src/heap.c \
    ../../src/misc.c \
    ../../src/polygn.c \
    ../../src/readvf.c \
    ../../src/savevf.c \
    ../../src/test3d.c \
    ../../src/tmpstore.c \
    ../../src/v3main.c \
    ../../src/view3d.c \
#    ../../src/viewht.c \
    ../../src/viewobs.c \
    ../../src/viewpp.c \
    ../../src/viewunob.c

HEADERS += \
    ../../src/prtyp.h \
    ../../src/tmpstore.h \
    ../../src/types.h \
    ../../src/vglob.h \
    ../../src/view3d.h \
    ../../src/vxtrn.h

DISTFILES += \
    ../../src/CMakeLists.txt \
    ../../src/Makefile \
    ../../src/Makefile.windows

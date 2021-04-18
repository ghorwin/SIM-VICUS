TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CFLAGS += -std=c89

SOURCES += \
    ../../../src/ctrans.c \
    ../../../src/getdat.c \
    ../../../src/heap.c \
    ../../../src/misc.c \
    ../../../src/polygn.c \
    ../../../src/readvf.c \
    ../../../src/savevf.c \
    ../../../src/test3d.c \
    ../../../src/tmpstore.c \
    ../../../src/v3main.c \
    ../../../src/view3d.c \
#    ../../../src/viewht.c \
    ../../../src/viewobs.c \
    ../../../src/viewpp.c \
    ../../../src/viewunob.c

HEADERS += \
    ../../../src/prtyp.h \
    ../../../src/tmpstore.h \
    ../../../src/types.h \
    ../../../src/vglob.h \
    ../../../src/view3d.h \
    ../../../src/vxtrn.h

DISTFILES += \
    ../../../src/CMakeLists.txt \
    ../../../src/Makefile \
    ../../../src/Makefile.windows

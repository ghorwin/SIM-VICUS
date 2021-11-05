# ---------------------------
# Project for penumbra library
# ---------------------------

# first we define what we are
TARGET = penumbra
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

# finally we setup our custom library specfic things
# like version number etc., we also may reset all
#
unix|mac {
                VER_MAJ = 1
                VER_MIN = 0
                VER_PAT = 0
                VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}



INCLUDEPATH +=	\

SOURCES += \
    ../../src/error.cpp \
    ../../src/gl/context.cpp \
    ../../src/gl/model.cpp \
    ../../src/gl/program.cpp \
    ../../src/gl/shader.cpp \
    ../../src/penumbra.cpp \
    ../../src/sun.cpp \
    ../../src/surface-private.cpp \
    ../../src/surface.cpp


HEADERS += \
    ../../src/KHR/khrplatform.h \
    ../../src/error.h \
    ../../src/gl/context.h \
    ../../src/gl/model.h \
    ../../src/gl/program.h \
    ../../src/gl/shader.h \
    ../../src/glad/glad.h \
    ../../src/glfw3.h \
    ../../src/glfw3native.h \
    ../../src/khrplatform.h \
    ../../src/linmath.h \
    ../../src/penumbra-private.h \
    ../../src/penumbra.h \
    ../../src/sun.h \
    ../../src/surface-private.h \
    ../../src/surface.h \
    ../../src/tesselator.h

DISTFILES += \
    ../../src/CMakeLists.txt



# ---------------------------
# Project for Shading library
# ---------------------------

# first we define what we are
TARGET = Shading
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

LIBS += -lDataIO -lCCM -lCCM -lTiCPP -lIBK -lIBKMK

INCLUDEPATH +=	\
		../../../IBK/src \
		../../../IBKMK/src \
		../../../CCM/src \
		../../../TiCPP/src \
		../../../DataIO/src

SOURCES += \
		../../src/SH_StructuralShading.cpp \
		../../src/SH_Polygon.cpp \
		../../src/SH_SunShadingAlgorithm.cpp


HEADERS += \
		../../src/SH_StructuralShading.h \
		../../src/SH_Polygon.h \
		../../src/SH_SunShadingAlgorithm.h


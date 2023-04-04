# ---------------------------
# Project for RoomClipper library
# ---------------------------
TARGET = RoomClipper
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

QT += gui core

unix|mac {
VER_MAJ = 0
	VER_MIN = 1
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lNandrad -lIBK -lIBKMK -lTiCPP -lCCM -lclipper -lVicus -llibdxfrw

INCLUDEPATH = \
../../src \
	../../../clipper/src \
	../../../IBK/src \
	../../../IBKMK/src \
	../../../CCM/src \
	../../../Nandrad/src \
	../../../DataIO/src \
	../../../Vicus/src \
	../../../TiCPP/src \
	../../../libdxfrw/src \
	../../../libdxfrw/src/intern

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
../../src/RC_Constants.cpp \
	../../src/RC_VicusClipping.cpp

HEADERS += \
../../src/RC_ClippingPolygon.h \
	../../src/RC_ClippingSurface.h \
	../../src/RC_Constants.h \
	../../src/RC_VicusClipping.h



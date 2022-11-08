# ---------------------------
# Project for Clipper library
# ---------------------------
TARGET = clipper
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

unix|mac {
	VER_MAJ = 6
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

INCLUDEPATH = \

DEPENDPATH = $$INCLUDEPATH

SOURCES += \
	../../src/clipper.cpp

HEADERS += \
	../../src/clipper.hpp


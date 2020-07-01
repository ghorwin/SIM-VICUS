# -------------------------------------------------
# Project for Nandrad library
# -------------------------------------------------
TARGET = Nandrad
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

unix|mac {
	VER_MAJ = 2
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lIBK -lTiCPP

INCLUDEPATH = \
	../../../IBK/src \
	../../../TiCPP/src

HEADERS += \
	../../src/NANDRAD_ArgsParser.h \
	../../src/NANDRAD_Constants.h

SOURCES += \
	../../src/NANDRAD_ArgsParser.cpp \
	../../src/NANDRAD_Constants.cpp

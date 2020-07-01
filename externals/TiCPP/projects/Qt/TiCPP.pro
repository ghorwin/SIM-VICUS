# -------------------------------------------------
# Project for TICPP library
# -------------------------------------------------
TARGET = TiCPP
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )


unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

SOURCES += \
	../../src/tinyxmlparser.cpp \
	../../src/tinyxmlerror.cpp \
	../../src/tinyxml.cpp \
	../../src/tinystr.cpp
HEADERS += \
	../../src/tinyxml.h \
	../../src/tinystr.h \
	../../src/ticppIBKconfig.h

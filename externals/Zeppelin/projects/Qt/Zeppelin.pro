# -------------------------------------------------
# Project for Zeppelin library
# -------------------------------------------------

TARGET = Zeppelin
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

SOURCES += ../../src/ZEPPELIN_DependencyGroup.cpp \
	../../src/ZEPPELIN_DependencyGraph.cpp

HEADERS += \
	../../src/ZEPPELIN_DependencyObject.h \
	../../src/ZEPPELIN_DependencyGroup.h \
	../../src/ZEPPELIN_DependencyGraph.h


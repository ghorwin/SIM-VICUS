# -------------------------------------------------
# Project for SIM-VICUS data model library
# -------------------------------------------------
TARGET = Vicus
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

unix|mac {
	VER_MAJ = 0
	VER_MIN = 1
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lNandrad -lIBK -lTiCPP

INCLUDEPATH = \
	../../src \
	../../../IBK/src \
	../../../Nandrad/src \
	../../../TiCPP/src

DEPENDPATH = $${INCLUDEPATH}

HEADERS += \
	../../src/VICUS_ArgsParser.h \
	../../src/VICUS_CodeGenMacros.h \
	../../src/VICUS_Constants.h \ \
	../../src/VICUS_Project.h

SOURCES += \
	../../src/VICUS_ArgsParser.cpp \
	../../src/VICUS_Constants.cpp \
	../../src/VICUS_Project.cpp


# -------------------------------------------------
# Project for SIM-VICUS data model library
# -------------------------------------------------
TARGET = Vicus
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )


QT += gui core widgets


unix|mac {
	VER_MAJ = 0
	VER_MIN = 1
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lNandrad -lIBK -lIBKMK -lTiCPP

INCLUDEPATH = \
	../../src \
	../../../IBK/src \
	../../../IBKMK/src \
	../../../Nandrad/src \
	../../../TiCPP/src

DEPENDPATH = $${INCLUDEPATH}

HEADERS += \
	../../src/VICUS_ArgsParser.h \
	../../src/VICUS_Building.h \
	../../src/VICUS_BuildingLevel.h \
	../../src/VICUS_CodeGenMacros.h \
	../../src/VICUS_Constants.h \ \
	../../src/VICUS_Edge.h \
	../../src/VICUS_KeywordList.h \
	../../src/VICUS_Line.h \
	../../src/VICUS_Network.h \
	../../src/VICUS_Node.h \
	../../src/VICUS_Project.h \
	../../src/VICUS_Room.h \
	../../src/VICUS_Surface.h

SOURCES += \
	../../src/VICUS_ArgsParser.cpp \
	../../src/VICUS_Constants.cpp \
	../../src/VICUS_Edge.cpp \
	../../src/VICUS_KeywordList.cpp \
	../../src/VICUS_Line.cpp \
	../../src/VICUS_Network.cpp \
	../../src/VICUS_Node.cpp \
	../../src/VICUS_Project.cpp \
	../../src/ncg/ncg_VICUS_Building.cpp \
	../../src/ncg/ncg_VICUS_Project.cpp


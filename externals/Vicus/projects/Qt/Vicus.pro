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
	../../src/VICUS_KeywordList.h \
	../../src/VICUS_Network.h \
	../../src/VICUS_NetworkEdge.h \
	../../src/VICUS_NetworkFluid.h \
	../../src/VICUS_NetworkLine.h \
	../../src/VICUS_NetworkNode.h \
	../../src/VICUS_NetworkPipe.h \
	../../src/VICUS_PlaneGeometry.h \
	../../src/VICUS_Project.h \
	../../src/VICUS_Room.h \
	../../src/VICUS_Surface.h \
    ../../src/VICUS_ViewSettings.h \
    ../../src/VICUS_WindowGlazing.h

SOURCES += \
	../../src/VICUS_ArgsParser.cpp \
	../../src/VICUS_Constants.cpp \
	../../src/VICUS_KeywordList.cpp \
	../../src/VICUS_Network.cpp \
	../../src/VICUS_NetworkEdge.cpp \
	../../src/VICUS_NetworkFluid.cpp \
	../../src/VICUS_NetworkLine.cpp \
	../../src/VICUS_NetworkNode.cpp \
	../../src/VICUS_NetworkPipe.cpp \
	../../src/VICUS_PlaneGeometry.cpp \
	../../src/VICUS_Project.cpp \
	../../src/VICUS_WindowGlazing.cpp \
	../../src/ncg/ncg_VICUS_Building.cpp \
	../../src/ncg/ncg_VICUS_BuildingLevel.cpp \
	../../src/ncg/ncg_VICUS_NetworkFluid.cpp \
	../../src/ncg/ncg_VICUS_NetworkPipe.cpp \
	../../src/ncg/ncg_VICUS_Project.cpp \
	../../src/ncg/ncg_VICUS_Room.cpp \
	../../src/ncg/ncg_VICUS_Surface.cpp \
	../../src/ncg/ncg_VICUS_ViewSettings.cpp


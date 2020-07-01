# Project file for NandradSolver

TARGET = NandradSolver
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT -= core gui

CONFIG += console
CONFIG -= app_bundle

LIBS += \
	-lNandradModel \
	-lWindowModel \
	-lWallModel \
	-lZeppelin \
	-lDelphinLight \
	-lMaterials \
	-lIntegratorFramework \
	-lDataIO \
	-lNandrad \
	-lIBKMK \
	-lIBK \
	-lCCM \
	-lTiCPP \
	-lsundials \
	-lSuiteSparse \
	-lzlib

unix|mac {
	LIBS += -ldl
	QMAKE_CXXFLAGS +=  -std=c++11
}


INCLUDEPATH = \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/IntegratorFramework/src \
	../../../externals/Zeppelin/src \
	../../../externals/Materials/src \
	../../../externals/Delphin/src \
	../../../externals/DataIO/src \
	../../../externals/Nandrad/src \
	../../../externals/NandradModel/src

SOURCES += \
	../../src/main.cpp

HEADERS +=

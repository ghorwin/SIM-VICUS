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
	-lIntegratorFramework \
	-lNandrad \
	-lIBKMK \
	-lZeppelin \
	-lDataIO \
	-lCCM \
	-lIBK \
	-lTiCPP \
	-lsundials \
	-lSuiteSparse
#	-lzlib

unix|mac {
	LIBS += -ldl
	QMAKE_CXXFLAGS +=  -std=c++11
}


INCLUDEPATH = \
	../../../externals/CCM/src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/IntegratorFramework/src \
	../../../externals/Zeppelin/src \
	../../../externals/DataIO/src \
	../../../externals/Nandrad/src \
	../../../externals/sundials/src/include

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
	../../src/NM_Directories.cpp \
	../../src/NM_NandradModel.cpp \
	../../src/main.cpp

HEADERS += \
	../../src/NM_Directories.h \
	../../src/NM_NandradModel.h

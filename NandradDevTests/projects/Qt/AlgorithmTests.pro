# Project file for NandradSolver
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = AlgorithmTests
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT -= core gui

CONFIG += console
CONFIG -= app_bundle

LIBS += \
	-lIBKMK \
	-lIBK

contains( OPTIONS, lapack ) {
	LIBS += -llapack
}

INCLUDEPATH = \
	../../src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
	../../src/main_triangulation.cpp

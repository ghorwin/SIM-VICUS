# Project file for NandradSolver
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

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
	-lIntegratorFramework \
	-lNandrad \
	-lDataIO \
	-lIBKMK \
	-lZeppelin \
	-lCCM \
	-lIBK \
	-lTiCPP \
	-lsundials \
	-lSuiteSparse

contains( OPTIONS, lapack ) {
	LIBS += -llapack
}

INCLUDEPATH = \
	../../src \
	../../../externals/CCM/src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/IntegratorFramework/src \
	../../../externals/Zeppelin/src \
	../../../externals/Nandrad/src \
	../../../externals/SuiteSparse/src/include \
	../../../externals/sundials/src/include

DEPENDPATH = $${INCLUDEPATH} \
	../../../externals/DataIO/src \
	../../../externals/TiCPP/src

SOURCES += \
#	../../../NandradDevTests/src/main.cpp \
	../../src/main.cpp

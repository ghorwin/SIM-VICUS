# ----------------------------------
# Project for NandradSolverFMI library
# ----------------------------------

TARGET = NandradSolverFMI
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../externals/IBK/projects/Qt/IBK.pri )

# mind top-level location
QMAKE_LIBDIR += ../../../externals/lib$${DIR_PREFIX}
LIBS += -L../../../externals/lib$${DIR_PREFIX}

QMAKE_LIBDIR -= ../../../lib$${DIR_PREFIX}
LIBS -= -L../../../lib$${DIR_PREFIX}

# no Qt support needed
QT -= core gui

# we are creating a shared library
CONFIG += shared

CONFIG(debug, debug|release) {
	windows {
		DLLDESTDIR = ../../../bin/debug$${DIR_PREFIX}
	}
	else {
		DESTDIR = ../../../bin/debug$${DIR_PREFIX}
	}
}
else {
	windows {
		DLLDESTDIR = ../../../bin/release$${DIR_PREFIX}
	}
	else {
		DESTDIR = ../../../bin/release$${DIR_PREFIX}
	}
}


unix|mac {
	VER_MAJ = 2
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

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

#win32:LIBS += -liphlpapi
win32:LIBS += -lshell32

INCLUDEPATH = \
	../../src \
	../../../externals/CCM/src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/IntegratorFramework/src \
	../../../externals/Zeppelin/src \
	../../../externals/Nandrad/src \
	../../../NandradSolver/src \
	../../../externals/SuiteSparse/src/include \
	../../../externals/sundials/src/include

SOURCES += \
	../../src/InstanceData.cpp \
	../../src/NandradModelFMU.cpp \
	../../src/fmi2common/fmi2Functions.cpp \
	../../src/fmi2common/InstanceDataCommon.cpp

HEADERS += \
	../../src/InstanceData.h \
	../../src/NandradModelFMU.h \
	../../src/fmi2common/fmi2Functions.h \
	../../src/fmi2common/fmi2Functions_complete.h \
	../../src/fmi2common/fmi2FunctionTypes.h \
	../../src/fmi2common/fmi2TypesPlatform.h \
	../../src/fmi2common/InstanceDataCommon.h

OTHER_FILES += \
	../../src/fmi2common/readme.txt

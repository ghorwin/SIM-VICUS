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
	-lIntegratorFramework \
	-lNandrad \
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
	../../src/fmi2common/InstanceDataCommon.cpp \
    ../../../NandradSolver/src/NM_AbstractModel.cpp \
    ../../../NandradSolver/src/NM_ConstructionBalanceModel.cpp \
    ../../../NandradSolver/src/NM_ConstructionStatesModel.cpp \
    ../../../NandradSolver/src/NM_DefaultModel.cpp \
    ../../../NandradSolver/src/NM_DefaultStateDependency.cpp \
    ../../../NandradSolver/src/NM_Directories.cpp \
    ../../../NandradSolver/src/NM_FMIInputOutput.cpp \
    ../../../NandradSolver/src/NM_HydraulicNetworkFlowElements.cpp \
    ../../../NandradSolver/src/NM_HydraulicNetworkModel.cpp \
    ../../../NandradSolver/src/NM_InternalLoadsModel.cpp \
    ../../../NandradSolver/src/NM_KeywordList.cpp \
    ../../../NandradSolver/src/NM_Loads.cpp \
    ../../../NandradSolver/src/NM_NandradModel.cpp \
    ../../../NandradSolver/src/NM_NaturalVentilationModel.cpp \
    ../../../NandradSolver/src/NM_OutputFile.cpp \
    ../../../NandradSolver/src/NM_OutputHandler.cpp \
    ../../../NandradSolver/src/NM_QuantityName.cpp \
    ../../../NandradSolver/src/NM_RoomBalanceModel.cpp \
    ../../../NandradSolver/src/NM_RoomRadiationLoadsModel.cpp \
    ../../../NandradSolver/src/NM_RoomStatesModel.cpp \
    ../../../NandradSolver/src/NM_Schedules.cpp \
    ../../../NandradSolver/src/NM_StateModelGroup.cpp \
    ../../../NandradSolver/src/NM_SteadyStateSolver.cpp \
    ../../../NandradSolver/src/NM_ThermalNetworkBalanceModel.cpp \
    ../../../NandradSolver/src/NM_ThermalNetworkFlowElements.cpp \
    ../../../NandradSolver/src/NM_ThermalNetworkPrivate.cpp \
    ../../../NandradSolver/src/NM_ThermalNetworkStatesModel.cpp \
    ../../../NandradSolver/src/NM_VectorValuedQuantity.cpp \
    ../../../NandradSolver/src/NM_VectorValuedQuantityIndex.cpp \
    ../../../NandradSolver/src/NM_WindowModel.cpp

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

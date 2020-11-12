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
	-lIntegratorFramework \
	-lNandrad \
	-lIBKMK \
	-lZeppelin \
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
win32{
	LIBS += -lshell32

#CONFIG(release, debug|release): LIBS += $$quote(C:/Program Files             (x86)/Microsoft SDKs/Windows/v7.1A/Lib/shell32.lib)
#else:win32:CONFIG(debug, debug|release): LIBS += $$quote(C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/shell32.lib)
}



INCLUDEPATH = \
	../../src \
	../../../externals/CCM/src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/IntegratorFramework/src \
	../../../externals/Zeppelin/src \
	../../../externals/Nandrad/src \
	../../../externals/TiCPP/src \
	../../../externals/sundials/src/include

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
#	../../../NandradDevTests/src/main.cpp \
	../../src/NM_WindowModel.cpp \
	../../src/main.cpp \
	../../src/NM_AbstractModel.cpp \
	../../src/NM_ConstructionBalanceModel.cpp \
	../../src/NM_ConstructionStatesModel.cpp \
	../../src/NM_DefaultModel.cpp \
	../../src/NM_DefaultStateDependency.cpp \
	../../src/NM_Directories.cpp \
	../../src/NM_FMIInputOutput.cpp \
	../../src/NM_KeywordList.cpp \
	../../src/NM_Loads.cpp \
	../../src/NM_NandradModel.cpp \
	../../src/NM_NaturalVentilationModel.cpp \
	../../src/NM_OutputFile.cpp \
	../../src/NM_OutputHandler.cpp \
	../../src/NM_QuantityName.cpp \
	../../src/NM_RoomBalanceModel.cpp \
	../../src/NM_RoomStatesModel.cpp \
	../../src/NM_Schedules.cpp \
	../../src/NM_StateModelGroup.cpp \
	../../src/NM_SteadyStateSolver.cpp \
	../../src/NM_VectorValuedQuantity.cpp \
	../../src/NM_VectorValuedQuantityIndex.cpp

HEADERS += \
	../../doc/NandradSolverMainPage.h \
	../../src/NM_AbstractModel.h \
	../../src/NM_AbstractStateDependency.h \
	../../src/NM_AbstractTimeDependency.h \
	../../src/NM_ConstructionBalanceModel.h \
	../../src/NM_ConstructionStatesModel.h \
	../../src/NM_DefaultModel.h \
	../../src/NM_DefaultStateDependency.h \
	../../src/NM_Directories.h \
	../../src/NM_FMIInputOutput.h \
	../../src/NM_InputReference.h \
	../../src/NM_KeywordList.h \
	../../src/NM_Loads.h \
	../../src/NM_NandradModel.h \
	../../src/NM_NaturalVentilationModel.h \
	../../src/NM_OutputFile.h \
	../../src/NM_OutputHandler.h \
	../../src/NM_Physics.h \
	../../src/NM_QuantityDescription.h \
	../../src/NM_QuantityName.h \
	../../src/NM_RoomBalanceModel.h \
	../../src/NM_RoomStatesModel.h \
	../../src/NM_Schedules.h \
	../../src/NM_StateModelGroup.h \
	../../src/NM_SteadyStateSolver.h \
	../../src/NM_ValueReference.h \
	../../src/NM_VectorValuedQuantity.h \
	../../src/NM_VectorValuedQuantityIndex.h \
	../../src/NM_WindowModel.h

DISTFILES +=

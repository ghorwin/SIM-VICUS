# -------------------------------------------------
# Project for Nandrad library
# -------------------------------------------------
TARGET = Nandrad
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

unix|mac {
	VER_MAJ = 2
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lIBK -lTiCPP

INCLUDEPATH = \
	../../../IBK/src \
	../../../TiCPP/src

DEPENDPATH = $${INCLUDEPATH}

HEADERS += \
	../../src/NANDRAD_AnnualSchedules.h \
	../../src/NANDRAD_ArgsParser.h \
	../../src/NANDRAD_Constants.h \
	../../src/NANDRAD_ConstructionInstance.h \
	../../src/NANDRAD_DailyCycle.h \
	../../src/NANDRAD_EmbeddedObject.h \
	../../src/NANDRAD_EmbeddedObjectWindow.h \
	../../src/NANDRAD_FindHelpers.h \
	../../src/NANDRAD_IDGroup.h \
	../../src/NANDRAD_Interface.h \
	../../src/NANDRAD_InterfaceAirFlow.h \
	../../src/NANDRAD_InterfaceHeatConduction.h \
	../../src/NANDRAD_InterfaceLongWaveEmission.h \
	../../src/NANDRAD_InterfaceSolarAbsorption.h \
	../../src/NANDRAD_InterfaceVaporDiffusion.h \
	../../src/NANDRAD_Interval.h \
	../../src/NANDRAD_KeywordList.h \
	../../src/NANDRAD_LinearSplineParameter.h \
	../../src/NANDRAD_Location.h \
	../../src/NANDRAD_ObjectList.h \
	../../src/NANDRAD_OutputDefinition.h \
	../../src/NANDRAD_OutputGrid.h \
	../../src/NANDRAD_Outputs.h \
	../../src/NANDRAD_Project.h \
	../../src/NANDRAD_ProjectInfo.h \
	../../src/NANDRAD_Schedule.h \
	../../src/NANDRAD_ScheduleGroup.h \
	../../src/NANDRAD_Schedules.h \
	../../src/NANDRAD_Sensor.h \
	../../src/NANDRAD_SimulationParameter.h \
	../../src/NANDRAD_SolverParameter.h \
	../../src/NANDRAD_SpaceType.h \
	../../src/NANDRAD_Utilities.h \
	../../src/NANDRAD_Zone.h

SOURCES += \
	../../src/NANDRAD_AnnualSchedules.cpp \
	../../src/NANDRAD_ArgsParser.cpp \
	../../src/NANDRAD_Constants.cpp \
	../../src/NANDRAD_ConstructionInstance.cpp \
	../../src/NANDRAD_DailyCycle.cpp \
	../../src/NANDRAD_EmbeddedObject.cpp \
	../../src/NANDRAD_EmbeddedObjectWindow.cpp \
	../../src/NANDRAD_IDGroup.cpp \
	../../src/NANDRAD_Interface.cpp \
	../../src/NANDRAD_InterfaceAirFlow.cpp \
	../../src/NANDRAD_InterfaceHeatConduction.cpp \
	../../src/NANDRAD_InterfaceLongWaveEmission.cpp \
	../../src/NANDRAD_InterfaceSolarAbsorption.cpp \
	../../src/NANDRAD_InterfaceVaporDiffusion.cpp \
	../../src/NANDRAD_Interval.cpp \
	../../src/NANDRAD_KeywordList.cpp \
	../../src/NANDRAD_LinearSplineParameter.cpp \
	../../src/NANDRAD_Location.cpp \
	../../src/NANDRAD_ObjectList.cpp \
	../../src/NANDRAD_OutputDefinition.cpp \
	../../src/NANDRAD_OutputGrid.cpp \
	../../src/NANDRAD_Outputs.cpp \
	../../src/NANDRAD_Project.cpp \
	../../src/NANDRAD_ProjectInfo.cpp \
	../../src/NANDRAD_Schedule.cpp \
	../../src/NANDRAD_ScheduleGroup.cpp \
	../../src/NANDRAD_Schedules.cpp \
	../../src/NANDRAD_Sensor.cpp \
	../../src/NANDRAD_SimulationParameter.cpp \
	../../src/NANDRAD_SolverParameter.cpp \
	../../src/NANDRAD_SpaceType.cpp \
	../../src/NANDRAD_Utilities.cpp \
	../../src/NANDRAD_Zone.cpp

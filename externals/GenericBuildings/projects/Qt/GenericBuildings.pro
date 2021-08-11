
# -------------------------------------------------
# Project for GenericBuildings library
# -------------------------------------------------
TARGET = GenericBuildings
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )


LIBS += \
		-lTiCPP \
		-lIBKMK \
		-lIBK

INCLUDEPATH = \
		../../../TiCPP/src \
		../../../IBK/src \
		../../../IBKMK/src

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
		../../src/EP_IDFParser.cpp \
	../../src/EP_ShadingBuildingDetailed.cpp \
		../../src/EP_Zone.cpp \
		../../src/EP_BuildingSurfaceDetailed.cpp \
		../../src/EP_Construction.cpp \
		../../src/EP_FenestrationSurfaceDetailed.cpp \
		../../src/EP_Material.cpp \
		../../src/EP_Project.cpp \
		../../src/EP_WindowMaterial.cpp \
#		../../src/NSG_Polygon.cpp \
#		../../src/NSG_PolygonWrapper.cpp \
#		../../src/NSG_SurfaceClipping.cpp

HEADERS += \
		../../src/EP_IDFParser.h \
	../../src/EP_ShadingBuildingDetailed.h \
	../../src/EP_Version.h \
		../../src/EP_Zone.h \
		../../src/EP_BuildingSurfaceDetailed.h \
		../../src/EP_Construction.h \
		../../src/EP_FenestrationSurfaceDetailed.h \
		../../src/EP_Material.h \
		../../src/EP_Project.h \
		../../src/EP_WindowMaterial.h \
#		../../src/NSG_Polygon.h \
#		../../src/NSG_PolygonWrapper.h \
#		../../src/NSG_SurfaceClipping.h

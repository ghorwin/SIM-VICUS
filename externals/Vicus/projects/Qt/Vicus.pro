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
	../../src/VICUS_AbstractDBElement.h \
	../../src/VICUS_ArgsParser.h \
	../../src/VICUS_BoundaryCondition.h \
	../../src/VICUS_Building.h \
	../../src/VICUS_BuildingLevel.h \
	../../src/VICUS_CodeGenMacros.h \
	../../src/VICUS_Component.h \
	../../src/VICUS_ComponentInstance.h \
	../../src/VICUS_Constants.h \ \
	../../src/VICUS_Construction.h \
	../../src/VICUS_Conversions.h \
	../../src/VICUS_Database.h \
	../../src/VICUS_EPDCategroySet.h \
	../../src/VICUS_EPDDataset.h \
	../../src/VICUS_KeywordList.h \
	../../src/VICUS_Material.h \
	../../src/VICUS_MaterialLayer.h \
	../../src/VICUS_Network.h \
	../../src/VICUS_NetworkComponent.h \
	../../src/VICUS_NetworkEdge.h \
	../../src/VICUS_NetworkFluid.h \
	../../src/VICUS_NetworkLine.h \
	../../src/VICUS_NetworkNode.h \
	../../src/VICUS_NetworkPipe.h \
	../../src/VICUS_Object.h \
	../../src/VICUS_Outputs.h \
	../../src/VICUS_PlaneGeometry.h \
	../../src/VICUS_Project.h \
	../../src/VICUS_Room.h \
	../../src/VICUS_RotationMatrix.h \
	../../src/VICUS_Surface.h \
	../../src/VICUS_SurfaceProperties.h \
	../../src/VICUS_ViewSettings.h \
	../../src/VICUS_Window.h \
	../../src/VICUS_WindowDivider.h \
	../../src/VICUS_WindowFrame.h \
	../../src/VICUS_WindowGlazingLayer.h \
	../../src/VICUS_WindowGlazingSystem.h

SOURCES += \
	../../src/VICUS_ArgsParser.cpp \
	../../src/VICUS_BoundaryCondition.cpp \
	../../src/VICUS_Component.cpp \
	../../src/VICUS_Constants.cpp \
	../../src/VICUS_Construction.cpp \
	../../src/VICUS_EPDCategroySet.cpp \
	../../src/VICUS_EPDDataset.cpp \
	../../src/VICUS_KeywordList.cpp \
	../../src/VICUS_Material.cpp \
	../../src/VICUS_Network.cpp \
	../../src/VICUS_NetworkComponent.cpp \
	../../src/VICUS_NetworkEdge.cpp \
	../../src/VICUS_NetworkFluid.cpp \
	../../src/VICUS_NetworkLine.cpp \
	../../src/VICUS_NetworkNode.cpp \
	../../src/VICUS_NetworkPipe.cpp \
	../../src/VICUS_Object.cpp \
	../../src/VICUS_Outputs.cpp \
	../../src/VICUS_PlaneGeometry.cpp \
	../../src/VICUS_Project.cpp \
	../../src/VICUS_Surface.cpp \
	../../src/ncg/ncg_VICUS_BoundaryCondition.cpp \
	../../src/ncg/ncg_VICUS_Building.cpp \
	../../src/ncg/ncg_VICUS_BuildingLevel.cpp \
	../../src/ncg/ncg_VICUS_Component.cpp \
	../../src/ncg/ncg_VICUS_ComponentInstance.cpp \
	../../src/ncg/ncg_VICUS_Construction.cpp \
	../../src/ncg/ncg_VICUS_EPDDataset.cpp \
	../../src/ncg/ncg_VICUS_Material.cpp \
	../../src/ncg/ncg_VICUS_MaterialLayer.cpp \
	../../src/ncg/ncg_VICUS_Network.cpp \
	../../src/ncg/ncg_VICUS_NetworkEdge.cpp \
	../../src/ncg/ncg_VICUS_NetworkFluid.cpp \
	../../src/ncg/ncg_VICUS_NetworkNode.cpp \
	../../src/ncg/ncg_VICUS_NetworkPipe.cpp \
	../../src/ncg/ncg_VICUS_Outputs.cpp \
	../../src/ncg/ncg_VICUS_Room.cpp \
	../../src/ncg/ncg_VICUS_Project.cpp \
	../../src/ncg/ncg_VICUS_RotationMatrix.cpp \
	../../src/ncg/ncg_VICUS_Surface.cpp \
	../../src/ncg/ncg_VICUS_SurfaceProperties.cpp \
	../../src/ncg/ncg_VICUS_ViewSettings.cpp \
	../../src/ncg/ncg_VICUS_Window.cpp \
	../../src/ncg/ncg_VICUS_WindowDivider.cpp \
	../../src/ncg/ncg_VICUS_WindowFrame.cpp \
	../../src/ncg/ncg_VICUS_WindowGlazingLayer.cpp \
	../../src/ncg/ncg_VICUS_WindowGlazingSystem.cpp

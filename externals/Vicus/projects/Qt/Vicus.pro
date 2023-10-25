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

LIBS += -lNandrad -lIBK -lIBKMK -lTiCPP -lCCM -lDataIO #-lHL

INCLUDEPATH = \
	../../src \
	../../../IBK/src \
	../../../IBKMK/src \
	../../../CCM/src \
	../../../Nandrad/src \
	../../../DataIO/src \
	#../../../HeatLoad/src \
	../../../TiCPP/src

DEPENDPATH = $${INCLUDEPATH}

HEADERS += \
        ../../src/VICUS_AbstractDBElement.h \
	../../src/VICUS_AcousticBoundaryCondition.h \
	../../src/VICUS_AcousticTemplate.h \
	../../src/VICUS_ArgsParser.h \
	../../src/VICUS_BTFReader.h \
	../../src/VICUS_BoundaryCondition.h \
	../../src/VICUS_Building.h \
	../../src/VICUS_BuildingLevel.h \
	../../src/VICUS_CodeGenMacros.h \
	../../src/VICUS_Component.h \
	../../src/VICUS_ComponentInstance.h \
	../../src/VICUS_Constants.h \ \
	../../src/VICUS_Construction.h \
	../../src/VICUS_DailyCycle.h \
	../../src/VICUS_Database.h \
	../../src/VICUS_Drawing.h \
	../../src/VICUS_DrawingLayer.h \
	../../src/VICUS_EmbeddedDatabase.h \
	../../src/VICUS_EpdCategorySet.h \
	../../src/VICUS_EpdDataset.h \
	../../src/VICUS_EpdModuleDataset.h \
	../../src/VICUS_GridPlane.h \
	../../src/VICUS_Infiltration.h \
	../../src/VICUS_InterfaceHeatConduction.h \
	../../src/VICUS_InternalLoad.h \
	../../src/VICUS_KeywordList.h \
	../../src/VICUS_LcaSettings.h \
	../../src/VICUS_LccSettings.h \
	../../src/VICUS_Material.h \
	../../src/VICUS_MaterialLayer.h \
	../../src/VICUS_Network.h \
	../../src/VICUS_NetworkBuriedPipeProperties.h \
	../../src/VICUS_NetworkComponent.h \
	../../src/VICUS_NetworkController.h \
	../../src/VICUS_NetworkEdge.h \
	../../src/VICUS_NetworkElement.h \
	../../src/VICUS_NetworkFluid.h \
	../../src/VICUS_NetworkLine.h \
	../../src/VICUS_NetworkNode.h \
	../../src/VICUS_NetworkPipe.h \
	../../src/VICUS_Object.h \
	../../src/VICUS_OutputDefinition.h \
	../../src/VICUS_Outputs.h \
	../../src/VICUS_PlainGeometry.h \
	../../src/VICUS_PlaneGeometry.h \
	../../src/VICUS_PlaneTriangulationData.h \
	../../src/VICUS_PolyLine.h \
	../../src/VICUS_Polygon2D.h \
	../../src/VICUS_Polygon3D.h \
	../../src/VICUS_Project.h \
	../../src/VICUS_Room.h \
	../../src/VICUS_RotationMatrix.h \
	../../src/VICUS_Schedule.h \
	../../src/VICUS_ScheduleInterval.h \
    	../../src/VICUS_SoundAbsorption.h \
	../../src/VICUS_SubNetwork.h \
	../../src/VICUS_SubSurface.h \
	../../src/VICUS_SubSurfaceComponent.h \
	../../src/VICUS_SubSurfaceComponentInstance.h \
	../../src/VICUS_SupplySystem.h \
	../../src/VICUS_Surface.h \
	../../src/VICUS_SurfaceHeating.h \
	../../src/VICUS_VentilationNatural.h \
	../../src/VICUS_ViewSettings.h \
	../../src/VICUS_Window.h \
	../../src/VICUS_WindowDivider.h \
	../../src/VICUS_WindowFrame.h \
	../../src/VICUS_WindowGlazingSystem.h \
	../../src/VICUS_ZoneControlNaturalVentilation.h \
	../../src/VICUS_ZoneControlShading.h \
	../../src/VICUS_ZoneControlThermostat.h \
	../../src/VICUS_ZoneIdealHeatingCooling.h \
	../../src/VICUS_ZoneTemplate.h \
	../../src/VICUS_utilities.h

SOURCES += \
        ../../src/VICUS_AbstractDBElement.cpp \
	../../src/VICUS_AcousticBoundaryCondition.cpp \
	../../src/VICUS_AcousticTemplate.cpp \
	../../src/VICUS_ArgsParser.cpp \
	../../src/VICUS_BTFReader.cpp \
	../../src/VICUS_BoundaryCondition.cpp \
	../../src/VICUS_Component.cpp \
	../../src/VICUS_Constants.cpp \
	../../src/VICUS_Construction.cpp \
	../../src/VICUS_DailyCycle.cpp \
	../../src/VICUS_Drawing.cpp \
	../../src/VICUS_DrawingLayer.cpp \
	../../src/VICUS_EpdCategorySet.cpp \
	../../src/VICUS_EpdDataset.cpp \
	../../src/VICUS_EpdModuleDataset.cpp \
	../../src/VICUS_GridPlane.cpp \
	../../src/VICUS_HeatLoad12831Export.cpp \
	../../src/VICUS_Infiltration.cpp \
	../../src/VICUS_InterfaceHeatConduction.cpp \
	../../src/VICUS_InternalLoad.cpp \
	../../src/VICUS_KeywordList.cpp \
	../../src/VICUS_LcaSettings.cpp \
	../../src/VICUS_LccSettings.cpp \
	../../src/VICUS_Material.cpp \
	../../src/VICUS_MaterialLayer.cpp \
	../../src/VICUS_Network.cpp \
	../../src/VICUS_NetworkBuriedPipeProperties.cpp \
	../../src/VICUS_NetworkComponent.cpp \
	../../src/VICUS_NetworkController.cpp \
	../../src/VICUS_NetworkEdge.cpp \
	../../src/VICUS_NetworkElement.cpp \
	../../src/VICUS_NetworkFluid.cpp \
	../../src/VICUS_NetworkLine.cpp \
	../../src/VICUS_NetworkNode.cpp \
	../../src/VICUS_NetworkPipe.cpp \
	../../src/VICUS_Object.cpp \
	../../src/VICUS_OutputDefinition.cpp \
	../../src/VICUS_Outputs.cpp \
	../../src/VICUS_PlainGeometry.cpp \
	../../src/VICUS_PlaneGeometry.cpp \
	../../src/VICUS_PolyLine.cpp \
	../../src/VICUS_Polygon2D.cpp \
	../../src/VICUS_Polygon3D.cpp \
	../../src/VICUS_Project.cpp \
	../../src/VICUS_ProjectGenerator.cpp \
	../../src/VICUS_Room.cpp \
	../../src/VICUS_Schedule.cpp \
	../../src/VICUS_ScheduleInterval.cpp \
    	../../src/VICUS_SoundAbsorption.cpp \
	../../src/VICUS_SubNetwork.cpp \
	../../src/VICUS_SubSurface.cpp \
	../../src/VICUS_SubSurfaceComponent.cpp \
	../../src/VICUS_SupplySystem.cpp \
	../../src/VICUS_Surface.cpp \
	../../src/VICUS_SurfaceHeating.cpp \
	../../src/VICUS_VentilationNatural.cpp \
	../../src/VICUS_Window.cpp \
	../../src/VICUS_WindowGlazingSystem.cpp \
	../../src/VICUS_ZoneControlNaturalVentilation.cpp \
	../../src/VICUS_ZoneControlShading.cpp \
	../../src/VICUS_ZoneControlThermostat.cpp \
	../../src/VICUS_ZoneIdealHeatingCooling.cpp \
	../../src/VICUS_ZoneTemplate.cpp \
	../../src/VICUS_utilities.cpp \
	../../src/ncg/ncg_VICUS_AcousticBoundaryCondition.cpp \
	../../src/ncg/ncg_VICUS_AcousticTemplate.cpp \
	../../src/ncg/ncg_VICUS_BoundaryCondition.cpp \
	../../src/ncg/ncg_VICUS_Building.cpp \
	../../src/ncg/ncg_VICUS_BuildingLevel.cpp \
	../../src/ncg/ncg_VICUS_Component.cpp \
	../../src/ncg/ncg_VICUS_ComponentInstance.cpp \
	../../src/ncg/ncg_VICUS_Construction.cpp \
	../../src/ncg/ncg_VICUS_DailyCycle.cpp \
	../../src/ncg/ncg_VICUS_Drawing.cpp \
	../../src/ncg/ncg_VICUS_DrawingLayer.cpp \
	../../src/ncg/ncg_VICUS_EmbeddedDatabase.cpp \
	../../src/ncg/ncg_VICUS_EpdDataset.cpp \
	../../src/ncg/ncg_VICUS_GridPlane.cpp \
	../../src/ncg/ncg_VICUS_Infiltration.cpp \
	../../src/ncg/ncg_VICUS_InterfaceHeatConduction.cpp \
	../../src/ncg/ncg_VICUS_InternalLoad.cpp \
	../../src/ncg/ncg_VICUS_LcaSettings.cpp \
	../../src/ncg/ncg_VICUS_LccSettings.cpp \
	../../src/ncg/ncg_VICUS_Material.cpp \
	../../src/ncg/ncg_VICUS_MaterialLayer.cpp \
	../../src/ncg/ncg_VICUS_Network.cpp \
	../../src/ncg/ncg_VICUS_NetworkBuriedPipeProperties.cpp \
	../../src/ncg/ncg_VICUS_NetworkComponent.cpp \
	../../src/ncg/ncg_VICUS_NetworkController.cpp \
	../../src/ncg/ncg_VICUS_NetworkEdge.cpp \
	../../src/ncg/ncg_VICUS_NetworkElement.cpp \
	../../src/ncg/ncg_VICUS_NetworkFluid.cpp \
	../../src/ncg/ncg_VICUS_NetworkNode.cpp \
	../../src/ncg/ncg_VICUS_NetworkPipe.cpp \
	../../src/ncg/ncg_VICUS_OutputDefinition.cpp \
	../../src/ncg/ncg_VICUS_Outputs.cpp \
	../../src/ncg/ncg_VICUS_PlainGeometry.cpp \
	../../src/ncg/ncg_VICUS_Project.cpp \
	../../src/ncg/ncg_VICUS_Room.cpp \
	../../src/ncg/ncg_VICUS_RotationMatrix.cpp \
	../../src/ncg/ncg_VICUS_Schedule.cpp \
	../../src/ncg/ncg_VICUS_ScheduleInterval.cpp \
	../../src/ncg/ncg_VICUS_SoundAbsorption.cpp \
	../../src/ncg/ncg_VICUS_SubNetwork.cpp \
	../../src/ncg/ncg_VICUS_SubSurface.cpp \
	../../src/ncg/ncg_VICUS_SubSurfaceComponent.cpp \
	../../src/ncg/ncg_VICUS_SubSurfaceComponentInstance.cpp \
	../../src/ncg/ncg_VICUS_SupplySystem.cpp \
	../../src/ncg/ncg_VICUS_Surface.cpp \
	../../src/ncg/ncg_VICUS_SurfaceHeating.cpp \
	../../src/ncg/ncg_VICUS_VentilationNatural.cpp \
	../../src/ncg/ncg_VICUS_ViewSettings.cpp \
	../../src/ncg/ncg_VICUS_Window.cpp \
	../../src/ncg/ncg_VICUS_WindowDivider.cpp \
	../../src/ncg/ncg_VICUS_WindowFrame.cpp \
	../../src/ncg/ncg_VICUS_WindowGlazingSystem.cpp \
	../../src/ncg/ncg_VICUS_ZoneControlNaturalVentilation.cpp \
	../../src/ncg/ncg_VICUS_ZoneControlShading.cpp \
	../../src/ncg/ncg_VICUS_ZoneControlThermostat.cpp \
	../../src/ncg/ncg_VICUS_ZoneIdealHeatingCooling.cpp \
	../../src/ncg/ncg_VICUS_ZoneTemplate.cpp

DISTFILES += \
	../../src/.gitignore

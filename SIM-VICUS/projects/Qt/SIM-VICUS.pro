# Project file for SIM-VICUS
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = SIM-VICUS
TEMPLATE = app

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT += xml opengl network printsupport widgets svg

CONFIG += c++11

unix {
	QMAKE_CXXFLAGS += -std=c++11 -Wno-deprecated-copy
}

LIBS += -L../../../lib$${DIR_PREFIX} \
	-lNandrad \
	-lQtExt \
	-lQuaZIP \
	-lTiCPP \
	-lIDFReader \
	-lVicus \
	-lShading \
	-lDataIO \
	-lCCM \
	-lIBK \
	-lIBKMK \
	-lsundials \
	-lSuiteSparse \
	-lqwt6

win32 {
	LIBS += -luser32
	LIBS += -lopengl32
}
linux {
	LIBS += -lGLU -lGL
}
mac {
	LIBS += -framework OpenGL
}

INCLUDEPATH = \
	../../src \
	../../src/actions \
	../../src/core3D \
	../../../externals/CCM/src \
	../../../externals/Shading/src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/Nandrad/src \
	../../../externals/Nandrad/srcTranslations \
	../../../externals/Vicus/src \
	../../../externals/Vicus/srcTranslations \
	../../../externals/TiCPP/src \
	../../../externals/qwt/src \
	../../../externals/QuaZIP/src \
	../../../externals/QuaZIP/src/zlib \
	../../../externals/IDFReader/src \
	../../../externals/QtExt/src

DEPENDPATH = $${INCLUDEPATH}

win32 {
PRE_TARGETDEPS += \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/IBK.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/CCM.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/Shading.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/QtExt.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/qwt6.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/QuaZIP.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/Vicus.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/Nandrad.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/TiCPP.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/IDFReader.lib \
	$$PWD/../../../externals/lib$${DIR_PREFIX}/IBKMK.lib
}


SOURCES += \
	../../../externals/Nandrad/srcTranslations/NANDRAD_KeywordListQt.cpp \
	../../../externals/Vicus/srcTranslations/VICUS_KeywordListQt.cpp \
	../../src/SVDBDialogAddDependentElements.cpp \
	../../src/SVDBDuplicatesDialog.cpp \
	../../src/SVDBNetworkControllerEditWidget.cpp \
	../../src/SVDBNetworkControllerTableModel.cpp \
	../../src/SVDBSubNetworkEditWidget.cpp \
	../../src/SVDBSubNetworkTableModel.cpp \
	../../src/SVOutputGridEditDialog.cpp \
	../../src/SVPreferencesPageMisc.cpp \
	../../src/SVPropBuildingBoundaryConditionsWidget.cpp \
	../../src/SVPropBuildingComponentOrientationWidget.cpp \
	../../src/SVPropBuildingComponentsWidget.cpp \
	../../src/SVPropBuildingSubComponentsWidget.cpp \
	../../src/SVPropBuildingSurfaceConnectionWidget.cpp \
	../../src/SVPropBuildingSurfaceHeatingWidget.cpp \
	../../src/SVPropBuildingZoneTemplatesWidget.cpp \
	../../src/SVPropSurfaceHeatingDelegate.cpp \
	../../src/SVSimulationNetworkOptions.cpp \
	../../src/SVSimulationOutputTableDelegate.cpp \
	../../src/SVSimulationOutputTableModel.cpp \
	../../src/SVSimulationShadingOptions.cpp \
	../../src/SVTimeSeriesPreviewWidget.cpp \
	../../src/SVZoneSelectionDialog.cpp \
	../../src/actions/SVUndoAddBuilding.cpp \
	../../src/actions/SVUndoAddBuildingLevel.cpp \
	../../src/actions/SVUndoAddNetwork.cpp \
	../../src/actions/SVUndoAddSurface.cpp \
	../../src/actions/SVUndoAddZone.cpp \
	../../src/actions/SVUndoCommandBase.cpp \
	../../src/actions/SVUndoCopySurfaces.cpp \
	../../src/actions/SVUndoCopyZones.cpp \
	../../src/actions/SVUndoDeleteBuilding.cpp \
	../../src/actions/SVUndoDeleteBuildingLevel.cpp \
	../../src/actions/SVUndoDeleteNetwork.cpp \
	../../src/actions/SVUndoDeleteSelected.cpp \
	../../src/actions/SVUndoModifyBuilding.cpp \
	../../src/actions/SVUndoModifyBuildingLevel.cpp \
	../../src/actions/SVUndoModifyBuildingTopology.cpp \
	../../src/actions/SVUndoModifyComponentInstances.cpp \
	../../src/actions/SVUndoModifyNetwork.cpp \
	../../src/actions/SVUndoModifyObjectName.cpp \
	../../src/actions/SVUndoModifyProject.cpp \
	../../src/actions/SVUndoModifyRoomZoneTemplateAssociation.cpp \
	../../src/actions/SVUndoModifySiteData.cpp \
	../../src/actions/SVUndoModifySubSurfaceComponentInstances.cpp \
	../../src/actions/SVUndoModifySurfaceGeometry.cpp \
	../../src/actions/SVUndoTreeNodeState.cpp \
	../../src/core3D/Vic3DCoordinateSystemObject.cpp \
	../../src/core3D/Vic3DGeometryHelpers.cpp \
	../../src/core3D/Vic3DGridObject.cpp \
	../../src/core3D/Vic3DKeyboardMouseHandler.cpp \
	../../src/core3D/Vic3DNewGeometryObject.cpp \
	../../src/core3D/Vic3DNewSubSurfaceObject.cpp \
	../../src/core3D/Vic3DOpaqueGeometryObject.cpp \
	../../src/core3D/Vic3DOpenGLWindow.cpp \
	../../src/core3D/Vic3DOrbitControllerObject.cpp \
	../../src/core3D/Vic3DPickObject.cpp \
	../../src/core3D/Vic3DScene.cpp \
	../../src/core3D/Vic3DSceneView.cpp \
	../../src/core3D/Vic3DShaderProgram.cpp \
	../../src/core3D/Vic3DSmallCoordinateSystemObject.cpp \
	../../src/core3D/Vic3DSurfaceNormalsObject.cpp \
	../../src/core3D/Vic3DTransform3D.cpp \
	../../src/core3D/Vic3DTransparentBuildingObject.cpp \
	../../src/core3D/Vic3DWireFrameObject.cpp \
	../../src/main.cpp \
	../../src/SVAboutDialog.cpp \
	../../src/SVAbstractDatabaseEditWidget.cpp \
	../../src/SVChartUtils.cpp \
	../../src/SVClimateDataSortFilterProxyModel.cpp \
	../../src/SVClimateDataTableModel.cpp \
	../../src/SVClimateFileInfo.cpp \
	../../src/SVConstants.cpp \
	../../src/SVDatabase.cpp \
	../../src/SVDatabaseEditDialog.cpp \
	../../src/SVDBBoundaryConditionEditWidget.cpp \
	../../src/SVDBBoundaryConditionTableModel.cpp \
	../../src/SVDBComponentEditWidget.cpp \
	../../src/SVDBComponentTableModel.cpp \
	../../src/SVDBConstructionEditWidget.cpp \
	../../src/SVDBConstructionTableModel.cpp \
	../../src/SVDBDailyCycleInputWidget.cpp \
	../../src/SVDBInfiltrationEditWidget.cpp \
	../../src/SVDBInfiltrationTableModel.cpp \
	../../src/SVDBInternalLoadsElectricEquipmentEditWidget.cpp \
	../../src/SVDBInternalLoadsLightsEditWidget.cpp \
	../../src/SVDBInternalLoadsOtherEditWidget.cpp \
	../../src/SVDBInternalLoadsPersonEditWidget.cpp \
	../../src/SVDBInternalLoadsTableModel.cpp \
	../../src/SVDBMaterialEditWidget.cpp \
	../../src/SVDBMaterialTableModel.cpp \
	../../src/SVDBModelDelegate.cpp \
	../../src/SVDBNetworkComponentEditWidget.cpp \
	../../src/SVDBNetworkComponentTableModel.cpp \
	../../src/SVDBNetworkFluidEditWidget.cpp \
	../../src/SVDBNetworkFluidTableModel.cpp \
	../../src/SVDBPipeEditWidget.cpp \
	../../src/SVDBPipeTableModel.cpp \
	../../src/SVDBScheduleAddDialog.cpp \
	../../src/SVDBScheduleDailyCycleEditWidget.cpp \
	../../src/SVDBScheduleEditWidget.cpp \
	../../src/SVDBScheduleTableModel.cpp \
	../../src/SVDBSubSurfaceComponentEditWidget.cpp \
	../../src/SVDBSubSurfaceComponentTableModel.cpp \
	../../src/SVDBSurfaceHeatingEditWidget.cpp \
	../../src/SVDBSurfaceHeatingTableModel.cpp \
	../../src/SVDBVentilationNaturalEditWidget.cpp \
	../../src/SVDBVentilationNaturalTableModel.cpp \
	../../src/SVDBWindowEditWidget.cpp \
	../../src/SVDBWindowGlazingSystemEditWidget.cpp \
	../../src/SVDBWindowGlazingSystemTableModel.cpp \
	../../src/SVDBWindowTableModel.cpp \
	../../src/SVDBZoneControlShadingEditWidget.cpp \
	../../src/SVDBZoneControlShadingTableModel.cpp \
	../../src/SVDBZoneControlThermostatEditWidget.cpp \
	../../src/SVDBZoneControlThermostatTableModel.cpp \
	../../src/SVDBZoneControlVentilationNaturalEditWidget.cpp \
	../../src/SVDBZoneControlVentilationNaturalTableModel.cpp \
	../../src/SVDBZoneIdealHeatingCoolingEditWidget.cpp \
	../../src/SVDBZoneIdealHeatingCoolingTableModel.cpp \
	../../src/SVDBZoneTemplateEditDialog.cpp \
	../../src/SVDBZoneTemplateEditWidget.cpp \
	../../src/SVDBZoneTemplateTreeModel.cpp \
	../../src/SVDebugApplication.cpp \
	../../src/SVGeometryView.cpp \
	../../src/SVImportIDFDialog.cpp \
	../../src/SVLCA.cpp \
	../../src/SVLocalCoordinateView.cpp \
	../../src/SVLogFileDialog.cpp \
	../../src/SVLogWidget.cpp \
	../../src/SVMainWindow.cpp \
	../../src/SVMessageHandler.cpp \
	../../src/SVNavigationTreeItemDelegate.cpp \
	../../src/SVNavigationTreeWidget.cpp \
	../../src/SVNetworkDialogSelectPipes.cpp \
	../../src/SVNetworkImportDialog.cpp \
	../../src/SVPostProcBindings.cpp \
	../../src/SVPostProcHandler.cpp \
	../../src/SVPreferencesDialog.cpp \
	../../src/SVPreferencesPageStyle.cpp \
	../../src/SVPreferencesPageTools.cpp \
	../../src/SVProjectHandler.cpp \
	../../src/SVPropAddWindowWidget.cpp \
	../../src/SVPropBuildingEditWidget.cpp \
	../../src/SVPropEditGeometry.cpp \
	../../src/SVPropertyWidget.cpp \
	../../src/SVPropFloorManagerItemDelegate.cpp \
	../../src/SVPropFloorManagerWidget.cpp \
	../../src/SVPropModeSelectionWidget.cpp \
	../../src/SVPropNetworkEditWidget.cpp \
	../../src/SVPropSiteWidget.cpp \
	../../src/SVPropVertexListWidget.cpp \
	../../src/SVScheduleHolidayWidget.cpp \
	../../src/SVSettings.cpp \
	../../src/SVSimulationLocationOptions.cpp \
	../../src/SVSimulationModelOptions.cpp \
	../../src/SVSimulationOutputOptions.cpp \
	../../src/SVSimulationPerformanceOptions.cpp \
	../../src/SVSimulationRunRequestDialog.cpp \
	../../src/SVSimulationStartNandrad.cpp \
	../../src/SVSmartSelectDialog.cpp \
	../../src/SVStyle.cpp \
	../../src/SVThreadBase.cpp \
	../../src/SVView3DDialog.cpp \
	../../src/SVViewState.cpp \
	../../src/SVViewStateHandler.cpp \
	../../src/SVWelcomeScreen.cpp

HEADERS  += \
	../../src/SVDBDialogAddDependentElements.h \
	../../src/SVDBDuplicatesDialog.h \
	../../src/SVDBNetworkControllerEditWidget.h \
	../../src/SVDBNetworkControllerTableModel.h \
	../../src/SVDBSubNetworkEditWidget.h \
	../../src/SVDBSubNetworkTableModel.h \
	../../src/SVOutputGridEditDialog.h \
	../../src/SVPreferencesPageMisc.h \
	../../src/SVPropBuildingBoundaryConditionsWidget.h \
	../../src/SVPropBuildingComponentOrientationWidget.h \
	../../src/SVPropBuildingComponentsWidget.h \
	../../src/SVPropBuildingSubComponentsWidget.h \
	../../src/SVPropBuildingSurfaceConnectionWidget.h \
	../../src/SVPropBuildingSurfaceHeatingWidget.h \
	../../src/SVPropBuildingZoneTemplatesWidget.h \
	../../src/SVPropSurfaceHeatingDelegate.h \
	../../src/SVSimulationNetworkOptions.h \
	../../src/SVSimulationOutputTableDelegate.h \
	../../src/SVSimulationOutputTableModel.h \
	../../src/SVSimulationShadingOptions.h \
	../../src/SVTimeSeriesPreviewWidget.h \
	../../src/SVZoneSelectionDialog.h \
	../../src/actions/SVUndoAddBuilding.h \
	../../src/actions/SVUndoAddBuildingLevel.h \
	../../src/actions/SVUndoAddNetwork.h \
	../../src/actions/SVUndoAddSurface.h \
	../../src/actions/SVUndoAddZone.h \
	../../src/actions/SVUndoCommandBase.h \
	../../src/actions/SVUndoCopySurfaces.h \
	../../src/actions/SVUndoCopyZones.h \
	../../src/actions/SVUndoDeleteBuilding.h \
	../../src/actions/SVUndoDeleteBuildingLevel.h \
	../../src/actions/SVUndoDeleteNetwork.h \
	../../src/actions/SVUndoDeleteSelected.h \
	../../src/actions/SVUndoModifyBuilding.h \
	../../src/actions/SVUndoModifyBuildingLevel.h \
	../../src/actions/SVUndoModifyBuildingTopology.h \
	../../src/actions/SVUndoModifyComponentInstances.h \
	../../src/actions/SVUndoModifyNetwork.h \
	../../src/actions/SVUndoModifyObjectName.h \
	../../src/actions/SVUndoModifyProject.h \
	../../src/actions/SVUndoModifyRoomZoneTemplateAssociation.h \
	../../src/actions/SVUndoModifySiteData.h \
	../../src/actions/SVUndoModifySubSurfaceComponentInstances.h \
	../../src/actions/SVUndoModifySurfaceGeometry.h \
	../../src/actions/SVUndoTreeNodeState.h \
	../../src/core3D/Vic3DCamera.h \
	../../src/core3D/Vic3DConstants.h \
	../../src/core3D/Vic3DCoordinateSystemObject.h \
	../../src/core3D/Vic3DGeometryHelpers.h \
	../../src/core3D/Vic3DGridObject.h \
	../../src/core3D/Vic3DKeyboardMouseHandler.h \
	../../src/core3D/Vic3DNewGeometryObject.h \
	../../src/core3D/Vic3DNewSubSurfaceObject.h \
	../../src/core3D/Vic3DOpaqueGeometryObject.h \
	../../src/core3D/Vic3DOpenGLException.h \
	../../src/core3D/Vic3DOpenGLWindow.h \
	../../src/core3D/Vic3DOrbitControllerObject.h \
	../../src/core3D/Vic3DPickObject.h \
	../../src/core3D/Vic3DScene.h \
	../../src/core3D/Vic3DSceneView.h \
	../../src/core3D/Vic3DShaderProgram.h \
	../../src/core3D/Vic3DSmallCoordinateSystemObject.h \
	../../src/core3D/Vic3DSurfaceNormalsObject.h \
	../../src/core3D/Vic3DTransform3D.h \
	../../src/core3D/Vic3DTransparentBuildingObject.h \
	../../src/core3D/Vic3DVertex.h \
	../../src/core3D/Vic3DWireFrameObject.h \
	../../src/SVAboutDialog.h \
	../../src/SVAbstractDatabaseEditWidget.h \
	../../src/SVChartUtils.h \
	../../src/SVClimateDataSortFilterProxyModel.h \
	../../src/SVClimateDataTableModel.h \
	../../src/SVClimateFileInfo.h \
	../../src/SVConstants.h \
	../../src/SVDatabaseEditDialog.h \
	../../src/SVDatabase.h \
	../../src/SVDBBoundaryConditionEditWidget.h \
	../../src/SVDBBoundaryConditionTableModel.h \
	../../src/SVDBComponentEditWidget.h \
	../../src/SVDBComponentTableModel.h \
	../../src/SVDBConstructionEditWidget.h \
	../../src/SVDBConstructionTableModel.h \
	../../src/SVDBDailyCycleInputWidget.h \
	../../src/SVDBInfiltrationEditWidget.h \
	../../src/SVDBInfiltrationTableModel.h \
	../../src/SVDBInternalLoadsElectricEquipmentEditWidget.h \
	../../src/SVDBInternalLoadsLightsEditWidget.h \
	../../src/SVDBInternalLoadsOtherEditWidget.h \
	../../src/SVDBInternalLoadsPersonEditWidget.h \
	../../src/SVDBInternalLoadsTableModel.h \
	../../src/SVDBMaterialEditWidget.h \
	../../src/SVDBMaterialTableModel.h \
	../../src/SVDBModelDelegate.h \
	../../src/SVDBNetworkComponentEditWidget.h \
	../../src/SVDBNetworkComponentTableModel.h \
	../../src/SVDBNetworkFluidEditWidget.h \
	../../src/SVDBNetworkFluidTableModel.h \
	../../src/SVDBPipeEditWidget.h \
	../../src/SVDBPipeTableModel.h \
	../../src/SVDBScheduleAddDialog.h \
	../../src/SVDBScheduleDailyCycleEditWidget.h \
	../../src/SVDBScheduleEditWidget.h \
	../../src/SVDBScheduleTableModel.h \
	../../src/SVDBSubSurfaceComponentEditWidget.h \
	../../src/SVDBSubSurfaceComponentTableModel.h \
	../../src/SVDBSurfaceHeatingEditWidget.h \
	../../src/SVDBSurfaceHeatingTableModel.h \
	../../src/SVDBVentilationNaturalEditWidget.h \
	../../src/SVDBVentilationNaturalTableModel.h \
	../../src/SVDBWindowEditWidget.h \
	../../src/SVDBWindowGlazingSystemEditWidget.h \
	../../src/SVDBWindowGlazingSystemTableModel.h \
	../../src/SVDBWindowTableModel.h \
	../../src/SVDBZoneControlShadingEditWidget.h \
	../../src/SVDBZoneControlShadingTableModel.h \
	../../src/SVDBZoneControlThermostatEditWidget.h \
	../../src/SVDBZoneControlThermostatTableModel.h \
	../../src/SVDBZoneControlVentilationNaturalEditWidget.h \
	../../src/SVDBZoneControlVentilationNaturalTableModel.h \
	../../src/SVDBZoneIdealHeatingCoolingEditWidget.h \
	../../src/SVDBZoneIdealHeatingCoolingTableModel.h \
	../../src/SVDBZoneTemplateEditDialog.h \
	../../src/SVDBZoneTemplateEditWidget.h \
	../../src/SVDBZoneTemplateTreeModel.h \
	../../src/SVDebugApplication.h \
	../../src/SVGeometryView.h \
	../../src/SVImportIDFDialog.h \
	../../src/SVLCA.h \
	../../src/SVLocalCoordinateView.h \
	../../src/SVLogFileDialog.h \
	../../src/SVLogWidget.h \
	../../src/SVMainWindow.h \
	../../src/SVMessageHandler.h \
	../../src/SVNavigationTreeItemDelegate.h \
	../../src/SVNavigationTreeWidget.h \
	../../src/SVNetworkDialogSelectPipes.h \
	../../src/SVNetworkImportDialog.h \
	../../src/SVPostProcBindings.h \
	../../src/SVPostProcHandler.h \
	../../src/SVPreferencesDialog.h \
	../../src/SVPreferencesPageStyle.h \
	../../src/SVPreferencesPageTools.h \
	../../src/SVProjectHandler.h \
	../../src/SVPropAddWindowWidget.h \
	../../src/SVPropBuildingEditWidget.h \
	../../src/SVPropEditGeometry.h \
	../../src/SVPropertyWidget.h \
	../../src/SVPropFloorManagerItemDelegate.h \
	../../src/SVPropFloorManagerWidget.h \
	../../src/SVPropModeSelectionWidget.h \
	../../src/SVPropNetworkEditWidget.h \
	../../src/SVPropSiteWidget.h \
	../../src/SVPropVertexListWidget.h \
	../../src/SVScheduleHolidayWidget.h \
	../../src/SVSettings.h \
	../../src/SVSimulationLocationOptions.h \
	../../src/SVSimulationModelOptions.h \
	../../src/SVSimulationOutputOptions.h \
	../../src/SVSimulationPerformanceOptions.h \
	../../src/SVSimulationRunRequestDialog.h \
	../../src/SVSimulationStartNandrad.h \
	../../src/SVSmartSelectDialog.h \
	../../src/SVStyle.h \
	../../src/SVThreadBase.h \
	../../src/SVView3DDialog.h \
	../../src/SVViewState.h \
	../../src/SVViewStateHandler.h \
	../../src/SVWelcomeScreen.h

FORMS    += \
	../../src/SVAboutDialog.ui \
	../../src/SVDBDialogAddDependentElements.ui \
	../../src/SVDBDuplicatesDialog.ui \
	../../src/SVDBNetworkControllerEditWidget.ui \
	../../src/SVDBSubNetworkEditWidget.ui \
	../../src/SVDatabaseEditDialog.ui \
	../../src/SVDBBoundaryConditionEditWidget.ui \
	../../src/SVDBComponentEditWidget.ui \
	../../src/SVDBConstructionEditWidget.ui \
	../../src/SVDBInfiltrationEditWidget.ui \
	../../src/SVDBInternalLoadsElectricEquipmentEditWidget.ui \
	../../src/SVDBInternalLoadsLightsEditWidget.ui \
	../../src/SVDBInternalLoadsOtherEditWidget.ui \
	../../src/SVDBInternalLoadsPersonEditWidget.ui \
	../../src/SVDBMaterialEditWidget.ui \
	../../src/SVDBNetworkComponentEditWidget.ui \
	../../src/SVDBNetworkFluidEditWidget.ui \
	../../src/SVDBPipeEditWidget.ui \
	../../src/SVDBScheduleAddDialog.ui \
	../../src/SVDBScheduleDailyCycleEditWidget.ui \
	../../src/SVDBScheduleEditWidget.ui \
	../../src/SVDBSubSurfaceComponentEditWidget.ui \
	../../src/SVDBSurfaceHeatingEditWidget.ui \
	../../src/SVDBVentilationNaturalEditWidget.ui \
	../../src/SVDBWindowEditWidget.ui \
	../../src/SVDBWindowGlazingSystemEditWidget.ui \
	../../src/SVDBZoneControlShadingEditWidget.ui \
	../../src/SVDBZoneControlThermostatEditWidget.ui \
	../../src/SVDBZoneControlVentilationNaturalEditWidget.ui \
	../../src/SVDBZoneIdealHeatingCoolingEditWidget.ui \
	../../src/SVDBZoneTemplateEditDialog.ui \
	../../src/SVDBZoneTemplateEditWidget.ui \
	../../src/SVImportIDFDialog.ui \
	../../src/SVLocalCoordinateView.ui \
	../../src/SVLogFileDialog.ui \
	../../src/SVMainWindow.ui \
	../../src/SVNavigationTreeWidget.ui \
	../../src/SVNetworkDialogSelectPipes.ui \
	../../src/SVNetworkImportDialog.ui \
	../../src/SVOutputGridEditDialog.ui \
	../../src/SVPreferencesDialog.ui \
	../../src/SVPreferencesPageMisc.ui \
	../../src/SVPreferencesPageStyle.ui \
	../../src/SVPreferencesPageTools.ui \
	../../src/SVPropAddWindowWidget.ui \
	../../src/SVPropBuildingBoundaryConditionsWidget.ui \
	../../src/SVPropBuildingComponentOrientationWidget.ui \
	../../src/SVPropBuildingComponentsWidget.ui \
	../../src/SVPropBuildingEditWidget.ui \
	../../src/SVPropBuildingSubComponentsWidget.ui \
	../../src/SVPropBuildingSurfaceConnectionWidget.ui \
	../../src/SVPropBuildingSurfaceHeatingWidget.ui \
	../../src/SVPropBuildingZoneTemplatesWidget.ui \
	../../src/SVPropEditGeometry.ui \
	../../src/SVPropFloorManagerWidget.ui \
	../../src/SVPropModeSelectionWidget.ui \
	../../src/SVPropNetworkEditWidget.ui \
	../../src/SVPropSiteWidget.ui \
	../../src/SVPropVertexListWidget.ui \
	../../src/SVScheduleHolidayWidget.ui \
	../../src/SVSimulationLocationOptions.ui \
	../../src/SVSimulationModelOptions.ui \
	../../src/SVSimulationNetworkOptions.ui \
	../../src/SVSimulationOutputOptions.ui \
	../../src/SVSimulationPerformanceOptions.ui \
	../../src/SVSimulationRunRequestDialog.ui \
	../../src/SVSimulationShadingOptions.ui \
	../../src/SVSimulationStartNandrad.ui \
	../../src/SVSmartSelectDialog.ui \
	../../src/SVWelcomeScreen.ui \
	../../src/SVZoneSelectionDialog.ui

TRANSLATIONS += ../../resources/translations/SIM-VICUS_de.ts
CODECFORSRC = UTF-8

RESOURCES += \
	../../resources/SIM-VICUS.qrc \
	../../resources/qdarkstyle/style.qrc \
	../../src/shaders/shaders.qrc






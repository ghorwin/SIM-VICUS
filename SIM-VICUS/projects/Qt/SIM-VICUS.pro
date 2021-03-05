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
				QMAKE_CXXFLAGS += -std=c++11
}

LIBS += -L../../../lib$${DIR_PREFIX} \
				-lCCM \
				-lIBK \
				-lIBKMK \
				-lNandrad \
				-lQtExt \
				-lQuaZIP \
				-lTiCPP \
				-lGenericBuildings \
				-lVicus \
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
				../../../externals/GenericBuildings/src \
				../../../externals/QtExt/src

DEPENDPATH = $${INCLUDEPATH}

win32 {
PRE_TARGETDEPS += \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/IBK.lib \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/CCM.lib \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/QtExt.lib \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/qwt6.lib \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/QuaZIP.lib \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/Vicus.lib \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/Nandrad.lib \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/TiCPP.lib \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/GenericBuildings.lib \
				$$PWD/../../../externals/lib$${DIR_PREFIX}/IBKMK.lib
}


SOURCES += \
				../../src/SVAbstractDatabaseEditWidget.cpp \
				../../src/SVChartUtils.cpp \
				../../src/SVDBDailyCycleInputWidget.cpp \
				../../src/SVDBInternalLoadsTableModel.cpp \
				../../src/SVDBScheduleAddDialog.cpp \
				../../src/SVDBInternalLoadsPersonEditDialog.cpp \
				../../src/SVDBInternalLoadsPersonEditWidget.cpp \
				../../src/SVDatabaseEditDialog.cpp \
				../../src/SVPropEditCopyDialog.cpp \
				../../src/main.cpp \
				../../../externals/Nandrad/srcTranslations/NANDRAD_KeywordListQt.cpp \
				../../../externals/Vicus/srcTranslations/VICUS_KeywordListQt.cpp \
				../../src/actions/SVUndoAddBuilding.cpp \
				../../src/actions/SVUndoAddBuildingLevel.cpp \
				../../src/actions/SVUndoAddNetwork.cpp \
				../../src/actions/SVUndoAddSurface.cpp \
				../../src/actions/SVUndoAddZone.cpp \
				../../src/actions/SVUndoCommandBase.cpp \
				../../src/actions/SVUndoCopyZones.cpp \
				../../src/actions/SVUndoDeleteSelected.cpp \
				../../src/actions/SVUndoModifyBuilding.cpp \
				../../src/actions/SVUndoModifyBuildingLevel.cpp \
				../../src/actions/SVUndoModifyBuildingTopology.cpp \
				../../src/actions/SVUndoModifyComponentInstances.cpp \
				../../src/actions/SVUndoModifyProject.cpp \
				../../src/actions/SVUndoModifySiteData.cpp \
				../../src/actions/SVUndoModifySurfaceGeometry.cpp \
				../../src/core3D/Vic3DCoordinateSystemObject.cpp \
				../../src/core3D/Vic3DGeometryHelpers.cpp \
				../../src/core3D/Vic3DGridObject.cpp \
				../../src/core3D/Vic3DKeyboardMouseHandler.cpp \
				../../src/core3D/Vic3DNewGeometryObject.cpp \
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
				../../src/core3D/Vic3DWireFrameObject.cpp \
				../../src/SVAboutDialog.cpp \
				../../src/SVClimateDataSortFilterProxyModel.cpp \
				../../src/SVClimateDataTableModel.cpp \
				../../src/SVClimateFileInfo.cpp \
				../../src/SVConstants.cpp \
				../../src/SVDatabase.cpp \
				../../src/SVDBBoundaryConditionEditDialog.cpp \
				../../src/SVDBBoundaryConditionEditWidget.cpp \
				../../src/SVDBBoundaryConditionTableModel.cpp \
				../../src/SVDBComponentEditWidget.cpp \
				../../src/SVDBComponentTableModel.cpp \
				../../src/SVDBConstructionEditWidget.cpp \
				../../src/SVDBConstructionTableModel.cpp \
				../../src/SVDBMaterialEditWidget.cpp \
				../../src/SVDBMaterialTableModel.cpp \
				../../src/SVDBModelDelegate.cpp \
				../../src/SVDBNetworkComponentEditDialog.cpp \
				../../src/SVDBNetworkComponentEditWidget.cpp \
				../../src/SVDBNetworkComponentTableModel.cpp \
				../../src/SVDBPipeEditDialog.cpp \
				../../src/SVDBPipeEditWidget.cpp \
				../../src/SVDBPipeTableModel.cpp \
				../../src/SVDBScheduleDailyCycleEditWidget.cpp \
				../../src/SVDBScheduleEditDialog.cpp \
				../../src/SVDBScheduleEditWidget.cpp \
				../../src/SVDBScheduleTableModel.cpp \
				../../src/SVDBWindowEditDialog.cpp \
				../../src/SVDBWindowEditWidget.cpp \
				../../src/SVDebugApplication.cpp \
				../../src/SVDialogSelectNetworkPipes.cpp \
				../../src/SVGeometryView.cpp \
				../../src/SVImportIDFDialog.cpp \
				../../src/SVInternalLoadsPersonDetailedWidget.cpp \
				../../src/SVInternalLoadsPersonManagerWidget.cpp \
				../../src/SVLCA.cpp \
				../../src/SVLocalCoordinateView.cpp \
				../../src/SVLogFileDialog.cpp \
				../../src/SVLogWidget.cpp \
				../../src/SVMainWindow.cpp \
				../../src/SVMessageHandler.cpp \
				../../src/SVNavigationTreeItemDelegate.cpp \
				../../src/SVNavigationTreeWidget.cpp \
				../../src/SVNetworkImportDialog.cpp \
				../../src/SVPostProcBindings.cpp \
				../../src/SVPostProcHandler.cpp \
				../../src/SVPreferencesDialog.cpp \
				../../src/SVPreferencesPageStyle.cpp \
				../../src/SVPreferencesPageTools.cpp \
				../../src/SVProjectHandler.cpp \
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
				../../src/SVSimulationStartNetworkSim.cpp \
				../../src/SVSmartSelectDialog.cpp \
				../../src/SVStyle.cpp \
				../../src/SVThreadBase.cpp \
				../../src/SVUndoCopySurfaces.cpp \
				../../src/SVUndoDeleteBuilding.cpp \
				../../src/SVUndoDeleteBuildingLevel.cpp \
				../../src/SVUndoDeleteNetwork.cpp \
				../../src/SVUndoModifyNetwork.cpp \
				../../src/SVUndoTreeNodeState.cpp \
				../../src/SVUtils.cpp \
				../../src/SVViewState.cpp \
				../../src/SVViewStateHandler.cpp \
				../../src/SVWelcomeScreen.cpp

HEADERS  += \
				../../src/SVAbstractDatabaseEditWidget.h \
				../../src/SVChartUtils.h \
				../../src/SVDBDailyCycleInputWidget.h \
				../../src/SVDBInternalLoadsTableModel.h \
				../../src/SVDBScheduleAddDialog.h \
				../../src/SVDBInternalLoadsPersonEditDialog.h \
				../../src/SVDBInternalLoadsPersonEditWidget.h \
				../../src/SVDatabaseEditDialog.h \
				../../src/SVPropEditCopyDialog.h \
				../../src/actions/SVUndoAddBuilding.h \
				../../src/actions/SVUndoAddBuildingLevel.h \
				../../src/actions/SVUndoAddNetwork.h \
				../../src/actions/SVUndoAddSurface.h \
				../../src/actions/SVUndoAddZone.h \
				../../src/actions/SVUndoCommandBase.h \
				../../src/actions/SVUndoCopyZones.h \
				../../src/actions/SVUndoDeleteSelected.h \
				../../src/actions/SVUndoModifyBuilding.h \
				../../src/actions/SVUndoModifyBuildingLevel.h \
				../../src/actions/SVUndoModifyBuildingTopology.h \
				../../src/actions/SVUndoModifyComponentInstances.h \
				../../src/actions/SVUndoModifyProject.h \
				../../src/actions/SVUndoModifySiteData.h \
				../../src/actions/SVUndoModifySurfaceGeometry.h \
				../../src/core3D/Vic3DCamera.h \
				../../src/core3D/Vic3DConstants.h \
				../../src/core3D/Vic3DCoordinateSystemObject.h \
				../../src/core3D/Vic3DGeometryHelpers.h \
				../../src/core3D/Vic3DGridObject.h \
				../../src/core3D/Vic3DKeyboardMouseHandler.h \
				../../src/core3D/Vic3DNewGeometryObject.h \
				../../src/core3D/Vic3DOpaqueGeometryObject.h \
				../../src/core3D/Vic3DOpenGLWindow.h \
				../../src/core3D/Vic3DOrbitControllerObject.h \
				../../src/core3D/Vic3DPickObject.h \
				../../src/core3D/Vic3DScene.h \
				../../src/core3D/Vic3DSceneView.h \
				../../src/core3D/Vic3DShaderProgram.h \
				../../src/core3D/Vic3DSmallCoordinateSystemObject.h \
				../../src/core3D/Vic3DSurfaceNormalsObject.h \
				../../src/core3D/Vic3DTransform3D.h \
				../../src/core3D/Vic3DVertex.h \
				../../src/core3D/Vic3DWireFrameObject.h \
				../../src/SVAboutDialog.h \
				../../src/SVClimateDataSortFilterProxyModel.h \
				../../src/SVClimateDataTableModel.h \
				../../src/SVClimateFileInfo.h \
				../../src/SVConstants.h \
				../../src/SVDatabase.h \
				../../src/SVDBBoundaryConditionEditDialog.h \
				../../src/SVDBBoundaryConditionEditWidget.h \
				../../src/SVDBBoundaryConditionTableModel.h \
				../../src/SVDBComponentEditWidget.h \
				../../src/SVDBComponentTableModel.h \
				../../src/SVDBConstructionEditWidget.h \
				../../src/SVDBConstructionTableModel.h \
				../../src/SVDBMaterialEditWidget.h \
				../../src/SVDBMaterialTableModel.h \
				../../src/SVDBModelDelegate.h \
				../../src/SVDBNetworkComponentEditDialog.h \
				../../src/SVDBNetworkComponentEditWidget.h \
				../../src/SVDBNetworkComponentTableModel.h \
				../../src/SVDBPipeEditDialog.h \
				../../src/SVDBPipeEditWidget.h \
				../../src/SVDBPipeTableModel.h \
				../../src/SVDBScheduleDailyCycleEditWidget.h \
				../../src/SVDBScheduleEditDialog.h \
				../../src/SVDBScheduleEditWidget.h \
				../../src/SVDBScheduleTableModel.h \
				../../src/SVDBWindowEditDialog.h \
				../../src/SVDBWindowEditWidget.h \
				../../src/SVDebugApplication.h \
				../../src/SVDialogSelectNetworkPipes.h \
				../../src/SVGeometryView.h \
				../../src/SVImportIDFDialog.h \
				../../src/SVInternalLoadsPersonDetailedWidget.h \
				../../src/SVInternalLoadsPersonManagerWidget.h \
				../../src/SVLCA.h \
				../../src/SVLocalCoordinateView.h \
				../../src/SVLogFileDialog.h \
				../../src/SVLogWidget.h \
				../../src/SVMainWindow.h \
				../../src/SVMessageHandler.h \
				../../src/SVNavigationTreeItemDelegate.h \
				../../src/SVNavigationTreeWidget.h \
				../../src/SVNetworkImportDialog.h \
				../../src/SVPostProcBindings.h \
				../../src/SVPostProcHandler.h \
				../../src/SVPreferencesDialog.h \
				../../src/SVPreferencesPageStyle.h \
				../../src/SVPreferencesPageTools.h \
				../../src/SVProjectHandler.h \
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
				../../src/SVSimulationStartNetworkSim.h \
				../../src/SVSmartSelectDialog.h \
				../../src/SVStyle.h \
				../../src/SVThreadBase.h \
				../../src/SVUndoCopySurfaces.h \
				../../src/SVUndoDeleteBuilding.h \
				../../src/SVUndoDeleteBuildingLevel.h \
				../../src/SVUndoDeleteNetwork.h \
				../../src/SVUndoModifyNetwork.h \
				../../src/SVUndoTreeNodeState.h \
				../../src/SVUtils.h \
				../../src/SVViewState.h \
				../../src/SVViewStateHandler.h \
				../../src/SVWelcomeScreen.h

FORMS    += \
				../../src/SVAboutDialog.ui \
				../../src/SVDBBoundaryConditionEditDialog.ui \
				../../src/SVDBBoundaryConditionEditWidget.ui \
				../../src/SVDBComponentEditWidget.ui \
				../../src/SVDBConstructionEditWidget.ui \
				../../src/SVDBInternalLoadsPersonEditDialog.ui \
				../../src/SVDBInternalLoadsPersonEditWidget.ui \
				../../src/SVDBMaterialEditWidget.ui \
				../../src/SVDBNetworkComponentEditDialog.ui \
				../../src/SVDBNetworkComponentEditWidget.ui \
				../../src/SVDBPipeEditDialog.ui \
				../../src/SVDBPipeEditWidget.ui \
				../../src/SVDBScheduleAddDialog.ui \
				../../src/SVDBScheduleDailyCycleEditWidget.ui \
				../../src/SVDBScheduleEditDialog.ui \
				../../src/SVDBScheduleEditWidget.ui \
				../../src/SVDBWindowEditDialog.ui \
				../../src/SVDBWindowEditWidget.ui \
				../../src/SVDatabaseEditDialog.ui \
				../../src/SVDialogSelectNetworkPipes.ui \
				../../src/SVImportIDFDialog.ui \
				../../src/SVInternalLoadsPersonDetailedWidget.ui \
				../../src/SVInternalLoadsPersonManagerWidget.ui \
				../../src/SVLocalCoordinateView.ui \
				../../src/SVLogFileDialog.ui \
				../../src/SVMainWindow.ui \
				../../src/SVNavigationTreeWidget.ui \
				../../src/SVNetworkImportDialog.ui \
				../../src/SVPreferencesDialog.ui \
				../../src/SVPreferencesPageStyle.ui \
				../../src/SVPreferencesPageTools.ui \
				../../src/SVPropBuildingEditWidget.ui \
				../../src/SVPropEditCopyDialog.ui \
				../../src/SVPropEditGeometry.ui \
				../../src/SVPropFloorManagerWidget.ui \
				../../src/SVPropModeSelectionWidget.ui \
				../../src/SVPropNetworkEditWidget.ui \
				../../src/SVPropSiteWidget.ui \
				../../src/SVPropVertexListWidget.ui \
				../../src/SVScheduleHolidayWidget.ui \
				../../src/SVSimulationLocationOptions.ui \
				../../src/SVSimulationModelOptions.ui \
				../../src/SVSimulationOutputOptions.ui \
				../../src/SVSimulationPerformanceOptions.ui \
				../../src/SVSimulationRunRequestDialog.ui \
				../../src/SVSimulationStartNandrad.ui \
				../../src/SVSimulationStartNetworkSim.ui \
				../../src/SVSmartSelectDialog.ui \
				../../src/SVWelcomeScreen.ui

TRANSLATIONS += ../../resources/translations/SIM-VICUS_de.ts
CODECFORSRC = UTF-8

RESOURCES += \
				../../resources/SIM-VICUS.qrc \
				../../resources/qdarkstyle/style.qrc \
				../../src/shaders/shaders.qrc






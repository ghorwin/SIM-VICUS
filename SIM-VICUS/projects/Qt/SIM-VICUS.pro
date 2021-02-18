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
								-lVicus

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
				../../../externals/QuaZIP/src \
				../../../externals/QuaZIP/src/zlib \
				../../../externals/GenericBuildings/src \
				../../../externals/QtExt/src

DEPENDPATH = $${INCLUDEPATH}

win32 {
				PRE_TARGETDEPS +=   $$PWD/../../../externals/lib$${DIR_PREFIX}/IBK.lib \
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
				../../../externals/Nandrad/srcTranslations/NANDRAD_KeywordListQt.cpp \
				../../../externals/Vicus/srcTranslations/VICUS_KeywordListQt.cpp \
				../../src/SVClimateDataTableModel.cpp \
				../../src/SVClimateFileInfo.cpp \
				../../src/SVDBBoundaryConditionEditDialog.cpp \
				../../src/SVDBBoundaryConditionEditWidget.cpp \
				../../src/SVDBBoundaryConditionTableModel.cpp \
				../../src/SVDBComponentEditDialog.cpp \
				../../src/SVDBComponentEditWidget.cpp \
				../../src/SVDBComponentTableModel.cpp \
				../../src/SVDBConstructionEditDialog.cpp \
				../../src/SVDBConstructionEditWidget.cpp \
				../../src/SVDBConstructionTableModel.cpp \
				../../src/SVDBMaterialEditDialog.cpp \
				../../src/SVDBMaterialEditWidget.cpp \
				../../src/SVDBMaterialTableModel.cpp \
				../../src/SVDBModelDelegate.cpp \
		../../src/SVDBNetworkComponentEditDialog.cpp \
		../../src/SVDBNetworkComponentEditWidget.cpp \
		../../src/SVDBNetworkComponentTableModel.cpp \
		../../src/SVDBPipeEditDialog.cpp \
		../../src/SVDBPipeEditWidget.cpp \
		../../src/SVDBPipeTableModel.cpp \
		../../src/SVDBWindowEditDialog.cpp \
				../../src/SVDatabase.cpp \
		../../src/SVDialogSelectNetworkPipes.cpp \
	../../src/SVInternalLoadsPersonDetailedWidget.cpp \
	../../src/SVInternalLoadsPersonManagerWidget.cpp \
		../../src/SVPropBuildingEditWidget.cpp \
	../../src/SVPropFloorManagerItemDelegate.cpp \
	../../src/SVPropFloorManagerWidget.cpp \
		../../src/SVPropModeSelectionWidget.cpp \
				../../src/SVPropNetworkEditWidget.cpp \
	../../src/SVScheduleEditWidget.cpp \
	../../src/SVScheduleHolidayWidget.cpp \
				../../src/SVSimulationLocationOptions.cpp \
				../../src/SVSimulationModelOptions.cpp \
				../../src/SVSimulationOutputOptions.cpp \
		../../src/SVSimulationRunRequestDialog.cpp \
		../../src/SVSmartSelectDialog.cpp \
	../../src/SVUndoDeleteBuilding.cpp \
	../../src/SVUndoDeleteBuildingLevel.cpp \
	../../src/SVUndoModifyNetwork.cpp \
				../../src/actions/SVUndoAddBuilding.cpp \
				../../src/actions/SVUndoAddBuildingLevel.cpp \
				../../src/actions/SVUndoAddNetwork.cpp \
				../../src/actions/SVUndoAddSurface.cpp \
				../../src/actions/SVUndoAddZone.cpp \
				../../src/actions/SVUndoCommandBase.cpp \
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
				../../src/main.cpp \
				../../src/SVAboutDialog.cpp \
				../../src/SVConstants.cpp \
				../../src/SVDBWindowEditWidget.cpp \
				../../src/SVDebugApplication.cpp \
				../../src/SVGeometryView.cpp \
				../../src/SVImportIDFDialog.cpp \
				../../src/SVLCA.cpp \
				../../src/SVLogFileDialog.cpp \
				../../src/SVLocalCoordinateView.cpp \
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
				../../src/SVPropEditGeometry.cpp \
				../../src/SVPropertyWidget.cpp \
				../../src/SVPropSiteWidget.cpp \
				../../src/SVPropVertexListWidget.cpp \
				../../src/SVSettings.cpp \
				../../src/SVSimulationPerformanceOptions.cpp \
				../../src/SVSimulationStartNandrad.cpp \
				../../src/SVSimulationStartNetworkSim.cpp \
				../../src/SVStyle.cpp \
				../../src/SVThreadBase.cpp \
				../../src/SVUndoDeleteNetwork.cpp \
				../../src/SVUndoTreeNodeState.cpp \
				../../src/SVUtils.cpp \
				../../src/SVViewState.cpp \
				../../src/SVViewStateHandler.cpp \
				../../src/SVWelcomeScreen.cpp \
	../../src/SVClimateDataSortFilterProxyModel.cpp

HEADERS  += \
				../../src/SVClimateDataTableModel.h \
				../../src/SVClimateFileInfo.h \
				../../src/SVDBBoundaryConditionEditDialog.h \
				../../src/SVDBBoundaryConditionEditWidget.h \
				../../src/SVDBBoundaryConditionTableModel.h \
				../../src/SVDBComponentEditDialog.h \
				../../src/SVDBComponentEditWidget.h \
				../../src/SVDBComponentTableModel.h \
				../../src/SVDBConstructionEditDialog.h \
				../../src/SVDBConstructionEditWidget.h \
				../../src/SVDBConstructionTableModel.h \
				../../src/SVDBMaterialEditDialog.h \
				../../src/SVDBMaterialEditWidget.h \
				../../src/SVDBMaterialTableModel.h \
				../../src/SVDBModelDelegate.h \
		../../src/SVDBNetworkComponentEditDialog.h \
		../../src/SVDBNetworkComponentEditWidget.h \
		../../src/SVDBNetworkComponentTableModel.h \
		../../src/SVDBPipeEditDialog.h \
		../../src/SVDBPipeEditWidget.h \
		../../src/SVDBPipeTableModel.h \
		../../src/SVDBWindowEditDialog.h \
				../../src/SVDatabase.h \
		../../src/SVDialogSelectNetworkPipes.h \
	../../src/SVInternalLoadsPersonDetailedWidget.h \
	../../src/SVInternalLoadsPersonManagerWidget.h \
		../../src/SVPropBuildingEditWidget.h \
	../../src/SVPropFloorManagerItemDelegate.h \
	../../src/SVPropFloorManagerWidget.h \
		../../src/SVPropModeSelectionWidget.h \
				../../src/SVPropNetworkEditWidget.h \
	../../src/SVScheduleEditWidget.h \
	../../src/SVScheduleHolidayWidget.h \
				../../src/SVSimulationLocationOptions.h \
				../../src/SVSimulationModelOptions.h \
				../../src/SVSimulationOutputOptions.h \
		../../src/SVSimulationRunRequestDialog.h \
		../../src/SVSmartSelectDialog.h \
	../../src/SVUndoDeleteBuilding.h \
	../../src/SVUndoDeleteBuildingLevel.h \
	../../src/SVUndoModifyNetwork.h \
				../../src/actions/SVUndoAddBuilding.h \
				../../src/actions/SVUndoAddBuildingLevel.h \
				../../src/actions/SVUndoAddNetwork.h \
				../../src/actions/SVUndoAddSurface.h \
				../../src/actions/SVUndoAddZone.h \
				../../src/actions/SVUndoCommandBase.h \
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
				../../src/SVConstants.h \
				../../src/SVDBWindowEditWidget.h \
				../../src/SVDebugApplication.h \
				../../src/SVGeometryView.h \
				../../src/SVImportIDFDialog.h \
				../../src/SVLCA.h \
				../../src/SVLogFileDialog.h \
				../../src/SVLocalCoordinateView.h \
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
				../../src/SVPropEditGeometry.h \
				../../src/SVPropertyWidget.h \
				../../src/SVPropSiteWidget.h \
				../../src/SVPropVertexListWidget.h \
				../../src/SVSettings.h \
				../../src/SVSimulationPerformanceOptions.h \
				../../src/SVSimulationStartNandrad.h \
				../../src/SVSimulationStartNetworkSim.h \
				../../src/SVStyle.h \
				../../src/SVThreadBase.h \
				../../src/SVUndoDeleteNetwork.h \
				../../src/SVUndoTreeNodeState.h \
				../../src/SVUtils.h \
				../../src/SVViewState.h \
				../../src/SVViewStateHandler.h \
				../../src/SVWelcomeScreen.h \
	../../src/SVClimateDataSortFilterProxyModel.h

FORMS    += \
				../../src/SVAboutDialog.ui \
				../../src/SVDBBoundaryConditionEditDialog.ui \
				../../src/SVDBBoundaryConditionEditWidget.ui \
				../../src/SVDBComponentEditDialog.ui \
				../../src/SVDBComponentEditWidget.ui \
				../../src/SVDBConstructionEditDialog.ui \
				../../src/SVDBConstructionEditWidget.ui \
				../../src/SVDBMaterialEditDialog.ui \
				../../src/SVDBMaterialEditWidget.ui \
				../../src/SVDBNetworkComponentEditDialog.ui \
				../../src/SVDBNetworkComponentEditWidget.ui \
				../../src/SVDBPipeEditDialog.ui \
				../../src/SVDBPipeEditWidget.ui \
				../../src/SVDBWindowEditDialog.ui \
				../../src/SVDBWindowEditWidget.ui \
				../../src/SVDialogSelectNetworkPipes.ui \
				../../src/SVImportIDFDialog.ui \
				../../src/SVInternalLoadsPersonDetailedWidget.ui \
				../../src/SVInternalLoadsPersonManagerWidget.ui \
				../../src/SVLogFileDialog.ui \
				../../src/SVLocalCoordinateView.ui \
				../../src/SVMainWindow.ui \
				../../src/SVNavigationTreeWidget.ui \
				../../src/SVNetworkImportDialog.ui \
				../../src/SVPreferencesDialog.ui \
				../../src/SVPreferencesPageStyle.ui \
				../../src/SVPreferencesPageTools.ui \
				../../src/SVPropBuildingEditWidget.ui \
				../../src/SVPropEditGeometry.ui \
				../../src/SVPropFloorManagerWidget.ui \
				../../src/SVPropModeSelectionWidget.ui \
				../../src/SVPropNetworkEditWidget.ui \
				../../src/SVPropSiteWidget.ui \
				../../src/SVPropVertexListWidget.ui \
				../../src/SVScheduleEditWidget.ui \
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

RESOURCES += ../../resources/SIM-VICUS.qrc \
				../../resources/qdarkstyle/style.qrc \
				../../src/shaders/shaders.qrc






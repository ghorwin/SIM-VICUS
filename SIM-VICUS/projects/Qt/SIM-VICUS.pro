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
		../../src/SVDatabase.cpp \
		../../src/SVDialogHydraulicComponents.cpp \
		../../src/SVPropNetworkEditWidget.cpp \
		../../src/SVSimulationLocationOptions.cpp \
		../../src/SVUndoDeleteNetworkHydraulicComponent.cpp \
		../../src/SVUndoModifyNetworkHydraulicComponent.cpp \
		../../src/actions/SVUndoAddBuilding.cpp \
		../../src/actions/SVUndoAddBuildingLevel.cpp \
		../../src/actions/SVUndoAddNetwork.cpp \
		../../src/actions/SVUndoAddSurface.cpp \
		../../src/actions/SVUndoCommandBase.cpp \
		../../src/actions/SVUndoDeleteSelected.cpp \
		../../src/actions/SVUndoModifySurfaceGeometry.cpp \
		../../src/actions/SVUndoProject.cpp \
		../../src/actions/SVUndoSiteDataChanged.cpp \
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
		../../src/SVLogWidget.cpp \
		../../src/SVMainWindow.cpp \
		../../src/SVMessageHandler.cpp \
		../../src/SVNavigationTreeItemDelegate.cpp \
		../../src/SVNavigationTreeWidget.cpp \
		../../src/SVNetworkEditDialog.cpp \
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
		../../src/SVUndoAddFluid.cpp \
		../../src/SVUndoDeleteNetwork.cpp \
		../../src/SVUndoModifyExistingNetwork.cpp \
		../../src/SVUndoTreeNodeState.cpp \
		../../src/SVUtils.cpp \
		../../src/SVViewState.cpp \
		../../src/SVViewStateHandler.cpp \
		../../src/SVWelcomeScreen.cpp

HEADERS  += \
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
		../../src/SVDatabase.h \
		../../src/SVDialogHydraulicComponents.h \
		../../src/SVPropNetworkEditWidget.h \
		../../src/SVSimulationLocationOptions.h \
		../../src/SVUndoDeleteNetworkHydraulicComponent.h \
		../../src/SVUndoModifyNetworkHydraulicComponent.h \
		../../src/actions/SVUndoAddBuilding.h \
		../../src/actions/SVUndoAddBuildingLevel.h \
		../../src/actions/SVUndoAddNetwork.h \
		../../src/actions/SVUndoAddSurface.h \
		../../src/actions/SVUndoCommandBase.h \
		../../src/actions/SVUndoDeleteSelected.h \
		../../src/actions/SVUndoModifySurfaceGeometry.h \
		../../src/actions/SVUndoProject.h \
		../../src/actions/SVUndoSiteDataChanged.h \
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
		../../src/SVLogWidget.h \
		../../src/SVMainWindow.h \
		../../src/SVMessageHandler.h \
		../../src/SVNavigationTreeItemDelegate.h \
		../../src/SVNavigationTreeWidget.h \
		../../src/SVNetworkEditDialog.h \
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
		../../src/SVUndoAddFluid.h \
		../../src/SVUndoDeleteNetwork.h \
		../../src/SVUndoModifyExistingNetwork.h \
		../../src/SVUndoTreeNodeState.h \
		../../src/SVUtils.h \
		../../src/SVViewState.h \
		../../src/SVViewStateHandler.h \
		../../src/SVWelcomeScreen.h

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
		../../src/SVDBWindowEditWidget.ui \
		../../src/SVDialogHydraulicComponents.ui \
		../../src/SVImportIDFDialog.ui \
		../../src/SVLogFileDialog.ui \
		../../src/SVMainWindow.ui \
		../../src/SVNavigationTreeWidget.ui \
		../../src/SVNetworkEditDialog.ui \
		../../src/SVNetworkImportDialog.ui \
		../../src/SVPreferencesDialog.ui \
		../../src/SVPreferencesPageStyle.ui \
		../../src/SVPreferencesPageTools.ui \
		../../src/SVPropEditGeometry.ui \
		../../src/SVPropNetworkEditWidget.ui \
		../../src/SVPropSiteWidget.ui \
		../../src/SVPropVertexListWidget.ui \
		../../src/SVSimulationLocationOptions.ui \
		../../src/SVSimulationPerformanceOptions.ui \
		../../src/SVSimulationStartNandrad.ui \
		../../src/SVSimulationStartNetworkSim.ui \
		../../src/SVWelcomeScreen.ui

TRANSLATIONS += ../../resources/translations/SIM-VICUS_de.ts
CODECFORSRC = UTF-8

RESOURCES += ../../resources/SIM-VICUS.qrc \
		../../resources/qdarkstyle/style.qrc \
		../../src/shaders/shaders.qrc






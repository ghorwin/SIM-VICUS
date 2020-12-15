# Project file for SIM-VICUS
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = SIM-VICUS
TEMPLATE = app

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT += xml opengl network printsupport widgets

CONFIG += c++11

unix {
		QMAKE_CXXFLAGS += -std=c++11
}

LIBS += -L../../../lib$${DIR_PREFIX} \
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
		../../../externals/IBK/src \
		../../../externals/IBKMK/src \
		../../../externals/Nandrad/src \
		../../../externals/Nandrad/srcTranslations \
		../../../externals/Vicus/src \
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
	../../src/SVDBConstructionOpaqueEditWidget.cpp \
		../../src/SVDBMaterialsEditWidget.cpp \
		../../src/SVDBWindowEditWidget.cpp \
		../../src/SVImportIDFDialog.cpp \
		../../src/SVLCA.cpp \
		../../src/SVMaterialTransfer.cpp \
		../../src/SVNavigationTreeItemDelegate.cpp \
		../../src/SVNetworkEditDialog.cpp \
		../../src/SVPreferencesPageStyle.cpp \
		../../src/SVPropEditGeometry.cpp \
		../../src/SVPropSiteWidget.cpp \
		../../src/SVPropVertexListWidget.cpp \
		../../src/SVPropertyWidget.cpp \
	../../src/SVSimulationPerformanceOptions.cpp \
	../../src/SVSimulationStartNetworkSim.cpp \
		../../src/SVUndoAddFluid.cpp \
		../../src/SVUndoDeleteNetwork.cpp \
		../../src/SVUndoModifyExistingNetwork.cpp \
		../../src/SVUndoTreeNodeState.cpp \
		../../src/SVUtils.cpp \
		../../src/SVViewState.cpp \
		../../src/SVViewStateHandler.cpp \
		../../src/actions/SVUndoAddBuilding.cpp \
		../../src/actions/SVUndoAddNetwork.cpp \
		../../src/actions/SVUndoAddSurface.cpp \
		../../src/actions/SVUndoCommandBase.cpp \
		../../src/actions/SVUndoModifySurfaceGeometry.cpp \
		../../src/actions/SVUndoProject.cpp \
		../../src/actions/SVUndoSiteDataChanged.cpp \
		../../src/core3D/Vic3DCoordinateSystemObject.cpp \
		../../src/core3D/Vic3DGeometryHelpers.cpp \
		../../src/core3D/Vic3DGridObject.cpp \
		../../src/core3D/Vic3DKeyboardMouseHandler.cpp \
		../../src/core3D/Vic3DNewPolygonObject.cpp \
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
		../../src/SVDebugApplication.cpp \
		../../src/SVGeometryView.cpp \
		../../src/SVLogFileDialog.cpp \
		../../src/SVLogWidget.cpp \
		../../src/SVMainWindow.cpp \
		../../src/SVMessageHandler.cpp \
		../../src/SVNavigationTreeWidget.cpp \
		../../src/SVNetworkImportDialog.cpp \
		../../src/SVPostProcBindings.cpp \
		../../src/SVPostProcHandler.cpp \
		../../src/SVPreferencesDialog.cpp \
		../../src/SVPreferencesPageTools.cpp \
		../../src/SVProjectHandler.cpp \
		../../src/SVSettings.cpp \
		../../src/SVStyle.cpp \
		../../src/SVThreadBase.cpp \
		../../src/SVWelcomeScreen.cpp \
	../../src/SVSimulationStartNandrad.cpp

HEADERS  += \
	../../src/SVDBConstructionOpaqueEditWidget.h \
		../../src/SVDBMaterialsEditWidget.h \
		../../src/SVDBWindowEditWidget.h \
		../../src/SVImportIDFDialog.h \
		../../src/SVLCA.h \
		../../src/SVMaterialTransfer.h \
		../../src/SVNavigationTreeItemDelegate.h \
		../../src/SVNetworkEditDialog.h \
		../../src/SVPreferencesPageStyle.h \
		../../src/SVPropEditGeometry.h \
		../../src/SVPropSiteWidget.h \
		../../src/SVPropVertexListWidget.h \
		../../src/SVPropertyWidget.h \
	../../src/SVSimulationPerformanceOptions.h \
	../../src/SVSimulationStartNetworkSim.h \
		../../src/SVUndoAddFluid.h \
		../../src/SVUndoDeleteNetwork.h \
		../../src/SVUndoModifyExistingNetwork.h \
		../../src/SVUndoTreeNodeState.h \
		../../src/SVUtils.h \
		../../src/SVViewState.h \
		../../src/SVViewStateHandler.h \
		../../src/actions/SVUndoAddBuilding.h \
		../../src/actions/SVUndoAddNetwork.h \
		../../src/actions/SVUndoAddSurface.h \
		../../src/actions/SVUndoCommandBase.h \
		../../src/actions/SVUndoModifySurfaceGeometry.h \
		../../src/actions/SVUndoProject.h \
		../../src/actions/SVUndoSiteDataChanged.h \
		../../src/core3D/Vic3DCamera.h \
		../../src/core3D/Vic3DConstants.h \
		../../src/core3D/Vic3DCoordinateSystemObject.h \
		../../src/core3D/Vic3DGeometryHelpers.h \
		../../src/core3D/Vic3DGridObject.h \
		../../src/core3D/Vic3DKeyboardMouseHandler.h \
		../../src/core3D/Vic3DNewPolygonObject.h \
		../../src/core3D/Vic3DOpaqueGeometryObject.h \
		../../src/core3D/Vic3DOpenGLWindow.h \
		../../src/core3D/Vic3DOrbitControllerObject.h \
		../../src/core3D/Vic3DPickObject.h \
		../../src/core3D/Vic3DScene.h \
		../../src/core3D/Vic3DSceneView.h \
		../../src/core3D/Vic3DShaderProgram.h \
		../../src/core3D/Vic3DTransform3D.h \
		../../src/core3D/Vic3DVertex.h \
		../../src/SVAboutDialog.h \
		../../src/SVConstants.h \
		../../src/SVDebugApplication.h \
		../../src/SVGeometryView.h \
		../../src/SVLogFileDialog.h \
		../../src/SVLogWidget.h \
		../../src/SVMainWindow.h \
		../../src/SVMessageHandler.h \
		../../src/SVNavigationTreeWidget.h \
		../../src/SVNetworkImportDialog.h \
		../../src/SVPostProcBindings.h \
		../../src/SVPostProcHandler.h \
		../../src/SVPreferencesDialog.h \
		../../src/SVPreferencesPageTools.h \
		../../src/SVProjectHandler.h \
		../../src/SVSettings.h \
		../../src/SVStyle.h \
		../../src/SVThreadBase.h \
		../../src/SVWelcomeScreen.h \
		../../src/core3D/Vic3DWireFrameObject.h \
	../../src/SVSimulationStartNandrad.h

FORMS    += \
		../../src/SVAboutDialog.ui \
	../../src/SVDBConstructionOpaqueEditWidget.ui \
		../../src/SVDBMaterialsEditWidget.ui \
		../../src/SVDBWindowEditWidget.ui \
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
		../../src/SVPropSiteWidget.ui \
		../../src/SVPropVertexListWidget.ui \
	../../src/SVSimulationPerformanceOptions.ui \
	../../src/SVSimulationStartNetworkSim.ui \
		../../src/SVWelcomeScreen.ui \
	../../src/SVSimulationStartNandrad.ui

TRANSLATIONS += ../../resources/translations/SIM-VICUS_de.ts
CODECFORSRC = UTF-8

RESOURCES += ../../resources/SIM-VICUS.qrc \
		../../resources/qdarkstyle/style.qrc \
		../../src/shaders/shaders.qrc






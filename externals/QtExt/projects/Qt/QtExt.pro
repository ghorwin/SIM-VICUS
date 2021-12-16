# -------------------------
# Project for QtExt library
# -------------------------

TARGET = QtExt
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )


QT += gui core network widgets printsupport svg

unix|mac {
	VER_MAJ = 1
	VER_MIN = 2
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lIBK

INCLUDEPATH += ../../src/ \
		../../../IBK/src \
		../../../IBKMK/src

DEPENDPATH = $${INCLUDEPATH}

FORMS += \
	../../src/QtExt_AutoUpdateDialog.ui \
	../../src/QtExt_ConstructionViewWidget.ui \
	../../src/QtExt_DateTimeInputDialog.ui \
	../../src/QtExt_LanguageStringEditWidget1.ui \
	../../src/QtExt_LanguageStringEditWidget3.ui \
	../../src/QtExt_MaterialDatabaseSelectionWidget.ui

HEADERS += \
	../../src/QtExt_ActiveLabel.h \
	../../src/QtExt_AutoUpdateDialog.h \
	../../src/QtExt_AutoUpdater.h \
	../../src/QtExt_BrowseFilenameWidget.h \
	../../src/QtExt_ColorButton.h \
	../../src/QtExt_Constants.h \
	../../src/QtExt_ConstructionGraphicsScene.h \
	../../src/QtExt_ConstructionLayer.h \
	../../src/QtExt_ConstructionView.h \
	../../src/QtExt_ConstructionViewWidget.h \
	../../src/QtExt_Conversions.h \
	../../src/QtExt_DateTimeInputDialog.h \
	../../src/QtExt_Directories.h \
	../../src/QtExt_FeatureWidget.h \
	../../src/QtExt_FilterComboBox.h \
	../../src/QtExt_GraphicsRectItemWithHatch.h \
	../../src/QtExt_IconButton.h \
	../../src/QtExt_LanguageHandler.h \
	../../src/QtExt_LanguageStringEditWidget1.h \
	../../src/QtExt_LanguageStringEditWidget3.h \
	../../src/QtExt_Locale.h \
	../../src/QtExt_MaterialBase.h \
	../../src/QtExt_MaterialCategory.h \
	../../src/QtExt_MaterialDatabaseSelectionWidget.h \
	../../src/QtExt_MaterialDBItemDelegate.h \
	../../src/QtExt_MaterialTableModel.h \
	../../src/QtExt_MaterialTableProxyModel.h \
	../../src/QtExt_ParameterEdit.h \
	../../src/QtExt_RectHatchingFunctions.h \
	../../src/QtExt_Settings.h \
	../../src/QtExt_Style.h \
	../../src/QtExt_TextFrame.h \
	../../src/QtExt_ValidatingInputBase.h \
	../../src/QtExt_ValidatingLineEdit.h \
	../../src/QtExt_ValueInputComboBox.h \
	../../src/QtExt_configuration.h \
	../../src/QtExt_varianthelper.h

SOURCES += \
	../../src/QtExt_ActiveLabel.cpp \
	../../src/QtExt_AutoUpdateDialog.cpp \
	../../src/QtExt_AutoUpdater.cpp \
	../../src/QtExt_BrowseFilenameWidget.cpp \
	../../src/QtExt_ColorButton.cpp \
	../../src/QtExt_Constants.cpp \
	../../src/QtExt_ConstructionGraphicsScene.cpp \
	../../src/QtExt_ConstructionLayer.cpp \
	../../src/QtExt_ConstructionView.cpp \
	../../src/QtExt_ConstructionViewWidget.cpp \
	../../src/QtExt_Conversions.cpp \
	../../src/QtExt_DateTimeInputDialog.cpp \
	../../src/QtExt_Directories.cpp \
	../../src/QtExt_FeatureWidget.cpp \
	../../src/QtExt_FilterComboBox.cpp \
	../../src/QtExt_GraphicsRectItemWithHatch.cpp \
	../../src/QtExt_IconButton.cpp \
	../../src/QtExt_LanguageHandler.cpp \
	../../src/QtExt_LanguageStringEditWidget1.cpp \
	../../src/QtExt_LanguageStringEditWidget3.cpp \
	../../src/QtExt_Locale.cpp \
	../../src/QtExt_MaterialCategory.cpp \
	../../src/QtExt_MaterialDatabaseSelectionWidget.cpp \
	../../src/QtExt_MaterialDBItemDelegate.cpp \
	../../src/QtExt_MaterialTableModel.cpp \
	../../src/QtExt_MaterialTableProxyModel.cpp \
	../../src/QtExt_ParameterEdit.cpp \
	../../src/QtExt_RectHatchingFunctions.cpp \
	../../src/QtExt_Settings.cpp \
	../../src/QtExt_Style.cpp \
	../../src/QtExt_TextFrame.cpp \
	../../src/QtExt_ValidatingInputBase.cpp \
	../../src/QtExt_ValidatingLineEdit.cpp \
	../../src/QtExt_ValueInputComboBox.cpp

RESOURCES += \
	../../resources/QtExt.qrc

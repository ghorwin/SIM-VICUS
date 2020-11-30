# -------------------------
# Project for QtExt library
# -------------------------

TARGET = QtExt
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )


QT += gui core network widgets

unix|mac {
	VER_MAJ = 1
	VER_MIN = 2
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lIBK

INCLUDEPATH += ../../src/ \
		../../../IBK/src

DEPENDPATH = $${INCLUDEPATH}

FORMS += \
	../../src/QtExt_AutoUpdateDialog.ui \
	../../src/QtExt_MaterialDatabaseSelectionWidget.ui

HEADERS += \
	../../src/QtExt_ActiveLabel.h \
	../../src/QtExt_AutoUpdateDialog.h \
	../../src/QtExt_AutoUpdater.h \
	../../src/QtExt_BrowseFilenameWidget.h \
	../../src/QtExt_Constants.h \
	../../src/QtExt_Directories.h \
	../../src/QtExt_FeatureWidget.h \
	../../src/QtExt_IconButton.h \
	../../src/QtExt_LanguageHandler.h \
	../../src/QtExt_MaterialBase.h \
	../../src/QtExt_MaterialCategory.h \
	../../src/QtExt_MaterialDBItemDelegate.h \
	../../src/QtExt_MaterialDatabaseSelectionWidget.h \
	../../src/QtExt_MaterialTableModel.h \
	../../src/QtExt_MaterialTableProxyModel.h \
	../../src/QtExt_Settings.h \
	../../src/QtExt_Style.h

SOURCES += \
	../../src/QtExt_ActiveLabel.cpp \
	../../src/QtExt_AutoUpdateDialog.cpp \
	../../src/QtExt_AutoUpdater.cpp \
	../../src/QtExt_BrowseFilenameWidget.cpp \
	../../src/QtExt_Constants.cpp \
	../../src/QtExt_Directories.cpp \
	../../src/QtExt_FeatureWidget.cpp \
	../../src/QtExt_IconButton.cpp \
	../../src/QtExt_LanguageHandler.cpp \
	../../src/QtExt_MaterialCategory.cpp \
	../../src/QtExt_MaterialDBItemDelegate.cpp \
	../../src/QtExt_MaterialDatabaseSelectionWidget.cpp \
	../../src/QtExt_MaterialTableModel.cpp \
	../../src/QtExt_MaterialTableProxyModel.cpp \
	../../src/QtExt_Settings.cpp \
	../../src/QtExt_Style.cpp

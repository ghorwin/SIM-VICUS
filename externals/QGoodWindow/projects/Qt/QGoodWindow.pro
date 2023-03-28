# -------------------------
# Project for QtExt library
# -------------------------

TARGET = QGoodWindow
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

include(QGoodWindow.pri)

QT += gui core network widgets printsupport svg

unix|mac {
	VER_MAJ = 2
	VER_MIN = 3
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS +=

INCLUDEPATH += ../../src/ \


DEPENDPATH = $${INCLUDEPATH}

RESOURCES += \
    ../../src/qgoodwindow_style.qrc


FORMS += \
	
HEADERS += \

SOURCES += \

DISTFILES += \

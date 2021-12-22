# Project file for DummyImportPlugin

TEMPLATE      = lib
CONFIG       += plugin
QT           += widgets

# this pri must be sourced from all our applications
include( ../../../../externals/IBK/projects/Qt/IBK.pri )

INCLUDEPATH  += \
	../../../../SIM-VICUS/src \
	../../../../SIM-VICUS/src/plugins

HEADERS       = ../../src/DummyImportPlugin.h
SOURCES       = ../../src/DummyImportPlugin.cpp

TARGET        = $$qtLibraryTarget(SV_DummyImportPlugin)
DESTDIR       = ../../plugins

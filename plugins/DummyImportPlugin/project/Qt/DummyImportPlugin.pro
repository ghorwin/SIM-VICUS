# Project file for DummyImportPlugin

TEMPLATE      = lib
CONFIG       += plugin
QT           += widgets

# this pri must be sourced from all our applications
include( ../../../Qt/plugin.pri )

INCLUDEPATH  += \
	../../../../SIM-VICUS/src \
	../../../../SIM-VICUS/src/plugins

HEADERS       = ../../src/DummyImportPlugin.h
SOURCES       = ../../src/DummyImportPlugin.cpp

TARGET        = $$qtLibraryTarget(SVDummyImportPlugin)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += TRILIBRARY

SOURCES += \
		triangle.c \
		main.cpp

HEADERS += \
		triangle.h

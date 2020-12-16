TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
#CONFIG += qt

QT += gui core

INCLUDEPATH += ../src

DEFINES += CDT_USE_AS_COMPILED_LIBRARY

SOURCES += \
		../src/CDT.cpp \
		main.cpp

HEADERS += \
	../src/CDT.h \
	../src/CDT.hpp \
	../src/CDTUtils.h \
	../src/CDTUtils.hpp \
	../src/PointRTree.h \
	../src/predicates.h \
	../src/remove_at.hpp

TEMPLATE = lib

TARGET = libdxfrw

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

INCLUDEPATH += \

HEADERS += \
$$files(../../src/*.h, true) \
	$$files(../../src/intern/*.h, true)

SOURCES += \
$$files(../../src/*.cpp, true) \

QMAKE_CXXFLAGS += -Wall

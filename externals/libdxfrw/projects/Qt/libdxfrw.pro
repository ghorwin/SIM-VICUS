TEMPLATE = lib

TARGET = libdxfrw

INCLUDEPATH += \

HEADERS += \
    $$files(../../src/*.h, true) \
    $$files(../../src/intern/*.h, true)

SOURCES += \
    $$files(../../src/*.cpp, true) \

QMAKE_CXXFLAGS += -Wall -Woverloaded-virtual

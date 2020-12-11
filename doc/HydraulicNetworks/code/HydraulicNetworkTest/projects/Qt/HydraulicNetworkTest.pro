# Project file for HydraulicNetworkTest
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux


TARGET = HydraulicNetworkTest
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

QT -= core gui

CONFIG += console
CONFIG -= app_bundle

QMAKE_LIBDIR += ../../../lib$${DIR_PREFIX}
LIBS += -L../../../lib$${DIR_PREFIX}

LIBS += \
    -lIBK \
    -lSuiteSparse \

unix|mac {
    LIBS += -ldl
    QMAKE_CXXFLAGS +=  -std=c++11
}
win32{
    LIBS += -lshell32

#CONFIG(release, debug|release): LIBS += $$quote(C:/Program Files             (x86)/Microsoft SDKs/Windows/v7.1A/Lib/shell32.lib)
#else:win32:CONFIG(debug, debug|release): LIBS += $$quote(C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/shell32.lib)
}



INCLUDEPATH = \
    ../../src \
    ../../../IBK/src \
    ../../../SuiteSparse/src/AMD/Include \
    ../../../SuiteSparse/src/BTF/Include \
    ../../../SuiteSparse/src/COLAMD/Include \
    ../../../SuiteSparse/src/KLU/Include \
    ../../../SuiteSparse/src/SuiteSparse_config \

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
    ../../src/AbstractFlowElement.cpp \
    ../../src/IBKMK_DenseMatrix.cpp \
    ../../src/main.cpp \
    ../../src/Network.cpp \
    ../../src/PipeElement.cpp \
    ../../src/Pump.cpp \
    ../../src/IBKMKC_dense_matrix.c \
    ../../src/IBKMK_SparseMatrixCSR.cpp \
    ../../src/IBKMKC_sparse_matrix.c \
    ../../src/IBKMK_SparseMatrixPattern.cpp

HEADERS += \
DISTFILES += \
    ../../src/AbstractFlowElement.h \
    ../../src/IBKMKC_dense_matrix.h \
    ../../src/IBKMK_common_defines.h \
    ../../src/IBKMK_DenseMatrix.h \
    ../../src/Network.h \
    ../../src/PipeElement.h \
    ../../src/Pump.h \
    ../../src/IBKMKC_sparse_matrix.h \
    ../../src/IBKMK_SparseMatrixCSR.h \
    ../../src/IBKMK_SparseMatrixPattern.h

SUBDIRS += \
    ../../src/HydraulicNetworkTest.pro

DISTFILES += \
    ../../src/HydraulicNetworkTest.pro.user

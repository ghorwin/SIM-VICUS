TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../IBK/src

LIBS += -L../lib_x64 -lIBK

SOURCES += \
		AbstractFlowElement.cpp \
		IBKMKC_dense_matrix.c \
		IBKMK_DenseMatrix.cpp \
		Network.cpp \
		PipeElement.cpp \
		Pump.cpp \
		main.cpp

HEADERS += \
	AbstractFlowElement.h \
	IBKMKC_dense_matrix.h \
	IBKMK_DenseMatrix.h \
	IBKMK_common_defines.h \
	Network.h \
	PipeElement.h \
	Pump.h

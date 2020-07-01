# -------------------------
# Project for IBKMK library
# -------------------------

# first we define what we are
TARGET = IBKMK
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

# finally we setup our custom library specfic things
# like version number etc., we also may reset all
#
unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}


LIBS += 		-lIBK


INCLUDEPATH = \
	../../../IBK/src


SOURCES += \
	../../src/IBKMKC_sparse_matrix.c \
	../../src/IBKMKC_vector_operations.c \
	../../src/IBKMK_SparseMatrixCSR.cpp \
	../../src/IBKMK_SparseMatrixPattern.cpp \
	../../src/IBKMKC_ilut.c

HEADERS += \
	../../src/IBKMKC_ilut.h \
	../../src/IBKMKC_sparse_matrix.h \
	../../src/IBKMKC_vector_operations.h \
	../../src/IBKMK_SparseMatrix.h \
	../../src/IBKMK_SparseMatrixCSR.h \
	../../src/IBKMK_SparseMatrixPattern.h \
	../../src/IBKMK_Vector3D.h \
	../../src/IBKMK_common_defines.h

DISTFILES += \
	../../doc/IBKMK_mainpage \
	../../doc/LICENSE


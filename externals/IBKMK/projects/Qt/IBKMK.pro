# -------------------------
# Project for IBKMK library
# -------------------------

# first we define what we are
TARGET = IBKMK
TEMPLATE = lib

DEFINES += CDT_USE_AS_COMPILED_LIBRARY

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
	../../../IBK/src \
	../../src/CDT


SOURCES += \
	../../src/CDT/CDT.cpp \
	../../src/IBKMKC_dense_matrix.c \
	../../src/IBKMKC_sparse_matrix.c \
	../../src/IBKMKC_vector_operations.c \
	../../src/IBKMK_DenseMatrix.cpp \
	../../src/IBKMK_SparseMatrixCSR.cpp \
	../../src/IBKMK_SparseMatrixPattern.cpp \
	../../src/IBKMKC_ilut.c \
	../../src/IBKMK_Triangulation.cpp

HEADERS += \
	../../src/CDT/CDT.h \
	../../src/CDT/CDT.hpp \
	../../src/CDT/CDTUtils.h \
	../../src/CDT/CDTUtils.hpp \
	../../src/CDT/CDT_predicates.h \
	../../src/CDT/CDT_remove_at.hpp \
	../../src/CDT/cdt_export.h \
	../../src/IBKMKC_dense_matrix.h \
	../../src/IBKMKC_ilut.h \
	../../src/IBKMKC_sparse_matrix.h \
	../../src/IBKMKC_vector_operations.h \
	../../src/IBKMK_DenseMatrix.h \
	../../src/IBKMK_SparseMatrix.h \
	../../src/IBKMK_SparseMatrixCSR.h \
	../../src/IBKMK_SparseMatrixPattern.h \
	../../src/IBKMK_Triangulation.h \
	../../src/IBKMK_Vector3D.h \
	../../src/IBKMK_common_defines.h

DISTFILES += \
	../../doc/IBKMK_mainpage \
	../../doc/LICENSE


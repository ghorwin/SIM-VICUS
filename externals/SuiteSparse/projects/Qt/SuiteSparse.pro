# ----------------------------------------
# Project for SuiteSparse lib
# ----------------------------------------

TARGET  = SuiteSparse
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

unix|mac {
	VER_MAJ = 1
	VER_MIN = 3
	VER_PAT = 8
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

INCLUDEPATH += \
	../../src/include

HEADERS += \
	../../src/include/amd.h \
	../../src/include/amd_internal.h \
	../../src/include/btf.h \
	../../src/include/btf_internal.h \
	../../src/include/colamd.h \
	../../src/include/klu.h \
	../../src/include/klu_internal.h \
	../../src/include/klu_version.h \
	../../src/include/SuiteSparse_config.h

SOURCES += \
	../../src/AMD/Source/amd_1.c \
	../../src/AMD/Source/amd_2.c \
	../../src/AMD/Source/amd_aat.c \
	../../src/AMD/Source/amd_control.c \
	../../src/AMD/Source/amd_defaults.c \
	../../src/AMD/Source/amd_dump.c \
	../../src/AMD/Source/amd_global.c \
	../../src/AMD/Source/amd_info.c \
	../../src/AMD/Source/amd_order.c \
	../../src/AMD/Source/amd_post_tree.c \
	../../src/AMD/Source/amd_postorder.c \
	../../src/AMD/Source/amd_preprocess.c \
	../../src/AMD/Source/amd_valid.c \
	../../src/BTF/Source/btf_maxtrans.c \
	../../src/BTF/Source/btf_order.c \
	../../src/BTF/Source/btf_strongcomp.c \
	../../src/COLAMD/Source/colamd.c \
	../../src/KLU/Source/klu.c \
	../../src/KLU/Source/klu_analyze.c \
	../../src/KLU/Source/klu_analyze_given.c \
	../../src/KLU/Source/klu_defaults.c \
	../../src/KLU/Source/klu_diagnostics.c \
	../../src/KLU/Source/klu_dump.c \
	../../src/KLU/Source/klu_extract.c \
	../../src/KLU/Source/klu_factor.c \
	../../src/KLU/Source/klu_free_numeric.c \
	../../src/KLU/Source/klu_free_symbolic.c \
	../../src/KLU/Source/klu_kernel.c \
	../../src/KLU/Source/klu_memory.c \
	../../src/KLU/Source/klu_refactor.c \
	../../src/KLU/Source/klu_scale.c \
	../../src/KLU/Source/klu_solve.c \
	../../src/KLU/Source/klu_sort.c \
	../../src/KLU/Source/klu_tsolve.c \
	../../src/SuiteSparse_config/SuiteSparse_config.c

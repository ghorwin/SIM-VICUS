# ----------------------------------------
# Project for sundials lib
# Includes only those components that
# are needed by IntegratorFramework
# ----------------------------------------

TARGET  = sundials
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

unix|mac {
	VER_MAJ = 2
	VER_MIN = 7
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lSuiteSparse

#DEFINES += SUNDIALS_DUMP_JACOBIAN

INCLUDEPATH += \
	../../src/include \
	../../src/src/sundials \
	../../src/src/cvode \
	../../src/src/kinsol \
	../../src/src/nvec_ser \
	../../../SuiteSparse/src/include

HEADERS += \
	../../src/include/cvode/cvode_band.h \
	../../src/include/cvode/cvode_bandpre.h \
	../../src/include/cvode/cvode_bbdpre.h \
	../../src/include/cvode/cvode_btridiag.h \
	../../src/include/cvode/cvode_dense.h \
	../../src/include/cvode/cvode_diag.h \
	../../src/include/cvode/cvode_direct.h \
	../../src/include/cvode/cvode.h \
	../../src/include/cvode/cvode_serialization.h \
	../../src/include/cvode/cvode_spbcgs.h \
	../../src/include/cvode/cvode_spgmr.h \
	../../src/include/cvode/cvode_spils.h \
	../../src/include/cvode/cvode_sptfqmr.h \
	../../src/include/sundials/sundials_band.h \
	../../src/include/sundials/sundials_btridiag.h \
	../../src/include/sundials/sundials_config.h \
	../../src/include/sundials/sundials_dense.h \
	../../src/include/sundials/sundials_direct.h \
	../../src/include/sundials/sundials_fnvector.h \
	../../src/include/sundials/sundials_iterative.h \
	../../src/include/sundials/sundials_klu_impl.h \
	../../src/include/sundials/sundials_math.h \
	../../src/include/sundials/sundials_nvector.h \
	../../src/include/sundials/sundials_pcg.h \
	../../src/include/sundials/sundials_serialization.h \
	../../src/include/sundials/sundials_sparse.h \
	../../src/include/sundials/sundials_spbcgs.h \
	../../src/include/sundials/sundials_spfgmr.h \
	../../src/include/sundials/sundials_spgmr.h \
	../../src/include/sundials/sundials_sptfqmr.h \
	../../src/include/sundials/sundials_superlumt_impl.h \
	../../src/include/sundials/sundials_timer.h \
	../../src/include/sundials/sundials_types.h \
	../../src/src/cvode/cvode_bandpre_impl.h \
	../../src/src/cvode/cvode_bbdpre_impl.h \
	../../src/src/cvode/cvode_diag_impl.h \
	../../src/src/cvode/cvode_direct_impl.h \
	../../src/src/cvode/cvode_impl.h \
	../../src/src/cvode/cvode_spils_impl.h \
	../../src/include/nvector/nvector_serial.h \
	../../src/include/cvode/cvode_klu.h

SOURCES += \
	../../src/src/cvode/cvode_band.c \
	../../src/src/cvode/cvode_bandpre.c \
	../../src/src/cvode/cvode_bbdpre.c \
	../../src/src/cvode/cvode_btridiag.c \
	../../src/src/cvode/cvode.c \
	../../src/src/cvode/cvode_dense.c \
	../../src/src/cvode/cvode_diag.c \
	../../src/src/cvode/cvode_direct.c \
	../../src/src/cvode/cvode_io.c \
	../../src/src/cvode/cvode_serialization.c \
	../../src/src/cvode/cvode_spbcgs.c \
	../../src/src/cvode/cvode_spgmr.c \
	../../src/src/cvode/cvode_spils.c \
	../../src/src/cvode/cvode_sptfqmr.c \
	../../src/src/kinsol/kinsol.c \
	../../src/src/kinsol/kinsol_band.c \
	../../src/src/kinsol/kinsol_bbdpre.c \
	../../src/src/kinsol/kinsol_dense.c \
	../../src/src/kinsol/kinsol_direct.c \
	../../src/src/kinsol/kinsol_io.c \
	../../src/src/kinsol/kinsol_klu.c \
	../../src/src/kinsol/kinsol_sparse.c \
	../../src/src/kinsol/kinsol_spbcgs.c \
	../../src/src/kinsol/kinsol_spfgmr.c \
	../../src/src/kinsol/kinsol_spgmr.c \
	../../src/src/kinsol/kinsol_spils.c \
	../../src/src/kinsol/kinsol_sptfqmr.c \
	../../src/src/nvec_ser/nvector_serial.c \
	../../src/src/sundials/sundials_band.c \
	../../src/src/sundials/sundials_btridiag.c \
	../../src/src/sundials/sundials_dense.c \
	../../src/src/sundials/sundials_direct.c \
	../../src/src/sundials/sundials_iterative.c \
	../../src/src/sundials/sundials_math.c \
	../../src/src/sundials/sundials_nvector.c \
	../../src/src/sundials/sundials_serialization.c \
	../../src/src/sundials/sundials_spbcgs.c \
	../../src/src/sundials/sundials_spgmr.c \
	../../src/src/sundials/sundials_sptfqmr.c \
	../../src/src/sundials/sundials_timer.c \
	../../src/src/cvode/cvode_klu.c \
	../../src/src/sundials/sundials_sparse.c \
	../../src/src/sundials/sundials_spfgmr.c \
	../../src/src/cvode/cvode_sparse.c

contains( OPTIONS, lapack ) {
		SOURCES +=
	message(Enabling Lapack in Sundials)
}


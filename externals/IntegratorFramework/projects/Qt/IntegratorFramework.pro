# ----------------------------------------
# Project for Integrator Framework Library
# ----------------------------------------
TARGET  = IntegratorFramework
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lIBK \
		-lIBKMK \
		-lsundials \
		-lSuiteSparse

INCLUDEPATH += \
		../../../IBKMK/src \
		../../../IBK/src \
		../../../sundials/src/include

DEPENDPATH = $$INCLUDEPATH

HEADERS += \
	../../src/SOLFRA_Constants.h \
	../../src/SOLFRA_FMUModelInterface.h \
	../../src/SOLFRA_IntegratorErrorControlled.h \
	../../src/SOLFRA_IntegratorInterface.h \
	../../src/SOLFRA_IntegratorSundialsCVODE.h \
	../../src/SOLFRA_IntegratorSundialsCVODEImpl.h \
	../../src/SOLFRA_JacobianInterface.h \
	../../src/SOLFRA_JacobianSparse.h \
	../../src/SOLFRA_JacobianSparseCSR.h \
	../../src/SOLFRA_LESBiCGStab.h \
	../../src/SOLFRA_LESDense.h \
	../../src/SOLFRA_LESGMRES.h \
	../../src/SOLFRA_LESInterface.h \
	../../src/SOLFRA_LESInterfaceDirect.h \
	../../src/SOLFRA_LESInterfaceIterative.h \
	../../src/SOLFRA_LESKLU.h \
	../../src/SOLFRA_ModelInterface.h \
	../../src/SOLFRA_OutputScheduler.h \
	../../src/SOLFRA_PrecondILU.h \
	../../src/SOLFRA_PrecondILUT.h \
	../../src/SOLFRA_PrecondInterface.h \
	../../src/SOLFRA_SolverControlFramework.h \
	../../src/SOLFRA_SolverFeedback.h

SOURCES += \
	../../src/SOLFRA_Constants.cpp \
	../../src/SOLFRA_IntegratorErrorControlled.cpp \
	../../src/SOLFRA_IntegratorSundialsCVODE.cpp \
	../../src/SOLFRA_JacobianSparseCSR.cpp \
	../../src/SOLFRA_LESBiCGStab.cpp \
	../../src/SOLFRA_LESDense.cpp \
	../../src/SOLFRA_LESGMRES.cpp \
	../../src/SOLFRA_LESInterfaceDirect.cpp \
	../../src/SOLFRA_LESInterfaceIterative.cpp \
	../../src/SOLFRA_LESKLU.cpp \
	../../src/SOLFRA_OutputScheduler.cpp \
	../../src/SOLFRA_PrecondILU.cpp \
	../../src/SOLFRA_PrecondILUT.cpp \
	../../src/SOLFRA_PrecondInterface.cpp \
	../../src/SOLFRA_SolverControlFramework.cpp \
	../../src/SOLFRA_SolverFeedback.cpp






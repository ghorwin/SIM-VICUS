#############################################
#
# Actual evironment configuration routines (kept in repository)
#
#############################################

# This section contains all mpi specific settings. It can be enabled
# by adding the option 'mpi' to the CONFIG environment variable.
# It defines USE_MPICH which can be utilized to create MPI code inside
# your ordinary serial code.
#
# creates an environment for mpi compilation
# mpicc must be found via systems executable path
# linux install dir is set to /opt/mpich
# windows must be added
#

# this is the central configuration file for all IBK dependent libraries
# we check if this file was created by our build tool helper python what ever
!include( CONFIG.pri ){
	message( "No custom build options specified" )
}

CONFIG			+= silent
CONFIG			-= depend_includepath

CONFIG(release, debug|release) {
#	message( "Setting NDEBUG define" )
	DEFINES += NDEBUG
}

linux-g++ | linux-g++-64 | macx {

		# our code doesn't check errno after calling math functions
	# so it is perfectly safe to disable it in favor of better performance
	# use *= to uniquely assign option
	QMAKE_CXXFLAGS   *= -fno-math-errno
}

contains( OPTIONS, sanitize_checks ) {

	CONFIG(debug, debug|release) {
		CONFIG += sanitizer
		CONFIG += sanitize_address
		CONFIG += sanitize_undefined
	}

	linux-g++ | linux-g++-64 | macx {
		QMAKE_CXXFLAGS_DEBUG   *= -fsanitize=address -fno-omit-frame-pointer
	}
}


contains( OPTIONS, mpi ) {

	message(Setting up MPICH support.)

	#setup compiler
	QMAKE_CXX = mpicxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = mpicc

	# setup linker
	QMAKE_LFLAGS += -FPIC $$system(mpicxx --showme:link)
	QMAKE_CFLAGS += $$system(mpicc --showme:compile)
	QMAKE_CXXFLAGS += $$system(mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK
	QMAKE_CXXFLAGS_RELEASE += $$system(mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK

	# setup define for actual source code
	 DEFINES += USE_MPICH


	unix {
		INCLUDEPATH += "/opt/mpich/include"
	}

	win32 {
		INCLUDEPATH += "/opt/mpich/include"
	}
}


#
# This option eables explicit mpi + icc support
#
contains( OPTIONS, mpiICC ) {

	message(Setting up MPICH+ICC support.)

	#setup compiler
	QMAKE_CXX = /opt/mpich_icc/bin/mpicxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/mpich_icc/bin/mpicc

	# setup linker
	QMAKE_LFLAGS += -FPIC $$system(/opt/mpich_icc/bin/mpicxx --showme:link)
	QMAKE_CFLAGS += $$system(/opt/mpich_icc/bin/mpicc --showme:compile)
	QMAKE_CXXFLAGS += $$system(/opt/mpich_icc/bin/mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK
	QMAKE_CXXFLAGS_RELEASE += $$system(/opt/mpich_icc/bin/mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK

	# setup define for actual source code
	 DEFINES += USE_MPICH

	unix {
		INCLUDEPATH += "/opt/mpich_icc/include"
	}

	win32 {
		INCLUDEPATH += "/opt/mpich_icc/include"
	}

}



#
# This option eables explicit mpi + gcc support
#
contains( OPTIONS, mpiGCC ) {

	message(Setting up MPICH+GCC support.)

	#setup compiler
	QMAKE_CXX = /opt/mpich_gcc/bin/mpicxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/mpich_gcc/bin/mpicc

	# setup linker
	QMAKE_LFLAGS += -FPIC $$system(/opt/mpich_gcc/bin/mpicxx --showme:link)
	QMAKE_CFLAGS += $$system(/opt/mpich_gcc/bin/mpicc --showme:compile)
	QMAKE_CXXFLAGS += $$system(/opt/mpich_gcc/bin/mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK
	QMAKE_CXXFLAGS_RELEASE += $$system(/opt/mpich_gcc/bin/mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK

	# setup define for actual source code
	 DEFINES += USE_MPICH

	unix {
		INCLUDEPATH += "/opt/mpich_gcc/include"
	}

	win32 {
		INCLUDEPATH += "/opt/mpich_gcc/include"
	}

}


#
# openmp settings added for gcc and min-gw
#
contains( OPTIONS, openmp ) {

	message(Setting up OpenMP support)

	#setup linker and compiler flags
	CONFIG(debug, debug|release) {
		QMAKE_CFLAGS = -fopenmp -fPIC -march=core-avx-i -mtune=core-avx-i
	} else {
		QMAKE_CFLAGS = -fopenmp -fPIC -O3 -march=core-avx-i -mtune=core-avx-i #-Ofast
	}
	QMAKE_LFLAGS = $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS = $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS

	# all OpenMP implementations define a _OPENMP symbol
}
else {

#	message(No OpenMP support)
	unix {
#		message(OpenMP warnings disabled)
		QMAKE_CFLAGS += -Wno-unknown-pragmas
		QMAKE_CXXFLAGS += -Wno-unknown-pragmas
	}

}


#
# This option eables explicit vampire trace + mpi + icc support
#
contains( OPTIONS, vapireTraceICC ) {

	message(Setting up TRACE+ICC support.)

	#setup compiler
	QMAKE_CXX = /opt/vampirTrace_icc/bin/vtcxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/vampirTrace_icc/bin/vtcc

	QMAKE_CFLAGS += -DVTRACE
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS

	# setup define for actual source code (manual instrumentation)
	DEFINES += USE_VAMPIRE

	unix {
		INCLUDEPATH += "/opt/vampirTrace_icc/include /opt/vampirTrace_icc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_icc/lib \
#				-l \
	}

	win32 {
		INCLUDEPATH += "/opt/vampirTrace_icc/include  /opt/vampirTrace_icc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_icc/lib \
#				-l \
	}

}



#
# This option eables explicit vampire trace + mpi + gcc support
#
contains( OPTIONS, vapireTraceGCC ) {

	message(Setting up TRACE+GCC support.)

	#setup compiler
	QMAKE_CXX = /opt/vampirTrace_gcc/bin/vtcxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/vampirTrace_gcc/bin/vtcc

	QMAKE_CFLAGS += -DVTRACE
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS


	# setup define for actual source code (manual instrumentation)
	DEFINES += USE_VAMPIRE

	unix {
		INCLUDEPATH += "/opt/vampirTrace_gcc/include /opt/vampirTrace_gcc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_gcc/lib \
#				-l \
	}

	win32 {
		INCLUDEPATH += "/opt/vampirTrace_gcc/include  /opt/vampirTrace_gcc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_gcc/lib \
#				-l \
	}

}


contains( OPTIONS, openmpVapireTraceICC ){

	message(Setting up OPENMP+TRACE+ICC support.)

	#setup compiler
	QMAKE_CXX = /opt/vampirTrace_icc/bin/vtcxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/vampirTrace_icc/bin/vtcc

	QMAKE_CFLAGS += -fopenmp -fPIC -DVTRACE
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS

	# setup define for actual source code (manual instrumentation)
	DEFINES += USE_VAMPIRE

	unix {
		INCLUDEPATH += "/opt/vampirTrace_icc/include /opt/vampirTrace_icc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_icc/lib \
#				-l \
	}

	win32 {
		INCLUDEPATH += "/opt/vampirTrace_icc/include  /opt/vampirTrace_icc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_icc/lib \
#				-l \
	}
}


contains( OPTIONS, openmpVapireTraceGCC ){

	message(Setting up OPENMP+TRACE+GCC support.)

	#setup compiler
	QMAKE_CXX = /opt/vampirTrace_gcc/bin/vtcxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/vampirTrace_gcc/bin/vtcc

	QMAKE_CFLAGS += -fopenmp -fPIC -O2
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS

	# setup define for actual source code (manual instrumentation)
	DEFINES += USE_VAMPIRE

	unix {
		INCLUDEPATH += "/opt/vampirTrace_gcc/include /opt/vampirTrace_gcc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_gcc/lib \
#				-l \
	}

	win32 {
		INCLUDEPATH += "/opt/vampirTrace_gcc/include  /opt/vampirTrace_gcc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_gcc/lib \
#				-l \
	}

}

#
# openmp settings added for gcc and min-gw
#
contains( OPTIONS, gprof ) {

	message(Setting up gprof support.)

	#setup linker and compiler flags
	QMAKE_CFLAGS += -pg
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS

}


# check if 32 or 64 bit version and set prefix variable for using in output paths
greaterThan(QT_MAJOR_VERSION, 4) {
	contains(QT_ARCH, i386): {
		DIR_PREFIX =
	} else {
		DIR_PREFIX = _x64
	}
} else {
	DIR_PREFIX =
}


#
# *** Applications ***
#
# This section contains all application specific settings. It can be enabled
# by setting 'app' the TEMPLATE environment variable.
# It defines DESTDIR, OBJECTS_DIR
# Reset locally in when required.
equals(TEMPLATE,app) {

	CONFIG(debug, debug|release) {
		OBJECTS_DIR = debug$${DIR_PREFIX}
		DESTDIR = ../../../bin/debug$${DIR_PREFIX}
	}
	else {
		OBJECTS_DIR = release$${DIR_PREFIX}
		DESTDIR = ../../../bin/release$${DIR_PREFIX}
	}

	MOC_DIR = moc
	UI_DIR = ui

	win32-msvc* {
		QMAKE_CXXFLAGS += /wd4996
		QMAKE_CFLAGS += /wd4996
		DEFINES += _CRT_SECURE_NO_WARNINGS
	}
	else {
		QMAKE_CXXFLAGS += -std=c++11
	}

	QMAKE_LIBDIR += ../../../externals/lib$${DIR_PREFIX}
	LIBS += -L../../../externals/lib$${DIR_PREFIX}

	win32:LIBS += -liphlpapi
	win32:LIBS += -lshell32
}



#
# *** Libraries ***
#
# This section contains all library specific settings. It can be enabled
# by setting 'lib' the TEMPLATE environment variable.
# It defines DESTDIR, OBJECTS_DIR, DLLDESTDIR and sets shared in CONFIG
# variable to all libraries. Reset locally in when required.
equals(TEMPLATE,lib) {

#	message(Setting up ordinary library support.)
	QT -=	core gui

	CONFIG += warn_on

	# set this even in case of no Qt library compilation to get mocs/uis organized in subdirs
	MOC_DIR = moc
	UI_DIR = ui

# using of shared libs only for non MC compiler
# MS compiler needs explicite export statements in case of shared libs
	win32-msvc* {
		CONFIG += static
		DEFINES += NOMINMAX
		DEFINES += _CRT_SECURE_NO_WARNINGS
		CONFIG(debug, debug|release) {
			QMAKE_CXXFLAGS += /GS /RTC1
		}
	}
	else {
		CONFIG += shared
	}

# disable warning for unsafe functions if using MS compiler
	win32-msvc* {
		QMAKE_CXXFLAGS += /wd4996
		QMAKE_CFLAGS += /wd4996
	}
	else {
		QMAKE_CXXFLAGS += -std=c++11
	}

	DESTDIR = ../../../lib$${DIR_PREFIX}
	LIBS += -L../../../lib$${DIR_PREFIX}

	CONFIG(debug, debug|release) {
		OBJECTS_DIR = debug$${DIR_PREFIX}
		windows {
			contains( OPTIONS, top_level_libs ) {
				DLLDESTDIR = ../../../bin/debug$${DIR_PREFIX}
			}
			else {
				DLLDESTDIR = ../../../../bin/debug$${DIR_PREFIX}
			}
		}
	}
	else {
		OBJECTS_DIR = release$${DIR_PREFIX}
		windows {
			contains( OPTIONS, top_level_libs ) {
				DLLDESTDIR = ../../../bin/release$${DIR_PREFIX}
			}
			else {
				DLLDESTDIR = ../../../../bin/release$${DIR_PREFIX}
			}
		}
	}

	win32:LIBS += -lshell32
	win32:LIBS += -liphlpapi

}

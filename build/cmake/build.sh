#!/bin/bash


# Build script for building application and all dependend libraries

# Command line options:  
#   [reldeb|release|debug]		build type
#   [2 [1..n]]					cpu count
#   [gcc|icc]					compiler
#   [off|gprof]					gprof (includes gcc)
#   [off|threadChecker]			threadchecker (includes icc)
#   [off|omp]					openmp (gcc and icc)
#   [verbose]					enable cmake to call verbose makefiles
#   [lapack]					enable cmake to build with lapack support
#   [skip-test]                 does not execute test-init script after successful build
#   []

# path export for mac
export PATH=~/Qt/5.11.3/gcc_64/bin:~/Qt/5.11.3/clang_64/bin:$PATH

CMAKELISTSDIR=$(pwd)
BUILDDIR="bb"

# set defaults
CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
MAKE_CPUCOUNT="2"
BUILD_DIR_SUFFIX="gcc"
COMPILER=""
SKIP_TESTS="false"

# parse parameters, except gprof and threadchecker
for var in "$@"
do

    if [[ $var = *[[:digit:]]* ]];
    then
		MAKE_CPUCOUNT=$var
		echo "Using $MAKE_CPUCOUNT CPUs for compilation"
    fi
    
    if [[ $var = "omp"  ]]; 
    then
		CMAKE_OPTIONS="$CMAKE_OPTIONS -DUSE_OPENMP:BOOL=ON"
		echo "Using Open MP compile flags"
    fi

    if [[ $var = "debug"  ]]; 
    then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=Debug"
		echo "Debug build..."
    fi

    if [[ $var = "release"  ]]; 
    then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=Release"
		echo "Release build..."
    fi

    if [[ $var = "reldeb"  ]]; 
    then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
		echo "RelWithDebInfo build..."
    fi

    if [[ $var = "icc"  && $COMPILER = "" ]]; 
    then
		COMPILER="icc"
		BUILD_DIR_SUFFIX="icc"
		echo "Intel compiler build..."
	    # export intel compiler path
	    CMAKE_COMPILER_OPTIONS="-DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icc"
	  fi

    if [[ $var = "gcc"  && $COMPILER = "" ]]; 
    then
		COMPILER="gcc"
		BUILD_DIR_SUFFIX="gcc"
		echo "GCC compiler build..."
		CMAKE_COMPILER_OPTIONS=""
	  fi

    if [[ $var = "verbose"  ]]; 
  	then
		CMAKE_OPTIONS="-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"
	  fi
	  
    if [[ $var = "skip-test"  ]]; 
  	then
		SKIP_TESTS="true"
	  fi

    if [[ $var = "lapack"  ]]; 
    then
		CMAKE_OPTIONS="$CMAKE_OPTIONS -DLAPACK_ENABLE:BOOL=ON"
		echo "Building with lapack support for CVODE"
    fi
	  
done


# override compiler options
for var in "$@"
do

    if [[ $var = "gprof" ]];
    then
		COMPILER="gcc"
		BUILD_DIR_SUFFIX="gcc"
		CMAKE_COMPILER_OPTIONS="-DCMAKE_CXX_FLAGS="'-pg'" -DCMAKE_EXE_LINKER_FLAGS="'-pg'""
		echo "Gprof build, forcing GCC build..."
    fi

    if [[ $var = "threadChecker"  ]]; 
    then
		COMPILER="icc"
		BUILD_DIR_SUFFIX="icc"
		echo "Using Threadchecker, forcing Intel compiler build..."
	    # export intel compiler path
	    CMAKE_COMPILER_OPTIONS="-DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icc -DUSE_THREAD_CHECKER:BOOL=ON"
	fi

done


# create build dir if not exists
BUILDDIR=$BUILDDIR-$BUILD_DIR_SUFFIX
if [ ! -d $BUILDDIR ]; then
    mkdir -p $BUILDDIR
fi

cd $BUILDDIR && cmake $CMAKE_OPTIONS $CMAKE_BUILD_TYPE $CMAKE_COMPILER_OPTIONS $CMAKELISTSDIR && make -j$MAKE_CPUCOUNT && 
cd $CMAKELISTSDIR &&
mkdir -p ../../bin/release &&
echo "*** Copying SIM-VICUS to bin/release ***" &&
if [ -d $BUILDDIR/SIM-VICUS/SIM-VICUS.app ]
then
    rm -rf ../../bin/release/SIM-VICUS.app
    cp -r $BUILDDIR/SIM-VICUS/SIM-VICUS.app ../../bin/release/SIM-VICUS.app
else
    cp $BUILDDIR/SIM-VICUS/SIM-VICUS ../../bin/release/SIM-VICUS
fi


################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

######################################################################
# qmake internal options
######################################################################

CONFIG           += qt     
CONFIG           += warn_on
CONFIG           += no_keywords
CONFIG           += silent
CONFIG           -= depend_includepath

######################################################################
# release/debug mode
######################################################################

win32 {
    # On Windows you can't mix release and debug libraries.
    # The designer is built in release mode. If you like to use it
    # you need a release version. For your own application development you
    # might need a debug version. 
    # Enable debug_and_release + build_all if you want to build both.

    CONFIG           += debug_and_release
    CONFIG           += build_all
}
else {

    #CONFIG           += release
    CONFIG           += debug

    VER_MAJ           = $${QWT_VER_MAJ}
    VER_MIN           = $${QWT_VER_MIN}
    VER_PAT           = $${QWT_VER_PAT}
    VERSION           = $${QWT_VERSION}
}

linux-g++ | linux-g++-64 {

    # CONFIG           += separate_debug_info

    # --- optional warnings 

    # QMAKE_CXXFLAGS   *= -Wfloat-equal 
    # QMAKE_CXXFLAGS   *= -Wshadow 
    # QMAKE_CXXFLAGS   *= -Wpointer-arith 
    # QMAKE_CXXFLAGS   *= -Wconversion 
    # QMAKE_CXXFLAGS   *= -Wsign-compare 
    # QMAKE_CXXFLAGS   *= -Wsign-conversion 
    # QMAKE_CXXFLAGS   *= -Wlogical-op
    # QMAKE_CXXFLAGS   *= -Werror=format-security
    # QMAKE_CXXFLAGS   *= -Woverloaded-virtual
    # QMAKE_CXXFLAGS   *= -std=c++11

    # --- optional debug options

    # QMAKE_CXXFLAGS_DEBUG   *= -fsanitize=address -fno-omit-frame-pointer 
    # QMAKE_CXXFLAGS_DEBUG   *= -fsanitize=address -fno-omit-frame-pointer
    # QMAKE_CXXFLAGS_DEBUG   *= -fsanitize=address

    # --- optional optimzations
 
    # qwt code doesn't check errno after calling math functions
    # so it is perfectly safe to disable it in favor of better performance
    QMAKE_CXXFLAGS   *= -fno-math-errno 

    # qwt code doesn't rely ( at least intends not to do )
    # on an exact implementation of IEEE or ISO rules/specifications
	# QMAKE_CXXFLAGS   *= -funsafe-math-optimizations 

    # also enables -fno-math-errno and -funsafe-math-optimizations
    # QMAKE_CXXFLAGS   *= -ffast-math

    # QMAKE_CXXFLAGS_DEBUG  *= -Og # since gcc 4.8

    # QMAKE_CXXFLAGS_RELEASE  *= -O3
    # QMAKE_CXXFLAGS_RELEASE  *= -Ofast
    # QMAKE_CXXFLAGS_RELEASE  *= -Os

    # when using the gold linker ( Qt < 4.8 ) - might be 
    # necessary on non linux systems too
    #QMAKE_LFLAGS += -lrt
}

linux-clang {

    #QMAKE_CXXFLAGS_RELEASE  *= -O3
}

######################################################################
# paths for building qwt
######################################################################

MOC_DIR      = moc
RCC_DIR      = resources

!debug_and_release {

    # in case of debug_and_release object files
    # are built in the release and debug subdirectories
    OBJECTS_DIR       = obj
}

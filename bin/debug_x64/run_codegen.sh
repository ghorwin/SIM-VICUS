#!/bin/bash

LIB_BUILD_PATH=../../../build-SIM-VICUS-session-Desktop-Debug/externals/lib_x64/

LD_LIBRARY_PATH=$LIB_BUILD_PATH ./NandradCodeGenerator VICUS ../../externals/Vicus/src 1 VICUS ncg
LD_LIBRARY_PATH=$LIB_BUILD_PATH ./NandradCodeGenerator NANDRAD ../../externals/Nandrad/src 1 NANDRAD ncg

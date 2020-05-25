#!/bin/bash

ctime -begin stella_linux.ctm

SRC_DIR=src
BUILD_DIR=build
VENDOR_DIR=vendor

EXECUTABLE=stella

CXXFLAGS="-std=c++17 -g -DGLEW_NO_GLU -nostdlib -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector -fno-builtin -Wa,--noexecstack"  # -s
CXXFLAGS="-nostdlib -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector -fno-builtin -Wa,--noexecstack"
#LDFLAGS="-L$VENDOR_DIR/GLEW/lib -L$VENDOR_DIR/GLFW/lib -L$VENDOR_DIR/imgui/lib -lGLEW -lglfw3 -l:imgui.a -lGL -ldl -lm -lX11 -lpthread"
LDFLAGS="-L$VENDOR_DIR/GLEW/lib -lGLEW -lGL -lX11 -lm -msse4.1"
INCLUDES="-I$VENDOR_DIR/GLEW/include -I$VENDOR_DIR/GLFW/include -I$VENDOR_DIR/imgui -I$VENDOR_DIR/sci.h -I$VENDOR_DIR/stb -I$VENDOR_DIR/rnd"

mkdir -p $BUILD_DIR

g++ $CXXFLAGS $INCLUDES $SRC_DIR/linux_platform.s $SRC_DIR/linux_platform.cpp -o $BUILD_DIR/stella $LDFLAGS
STATUS=$?

ctime -end stella_linux.ctm $STATUS
exit $STATUS
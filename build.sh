#!/bin/bash

ctime -begin stella_linux.ctm

SRC_DIR=src
BUILD_DIR=build
VENDOR_DIR=vendor

EXECUTABLE=stella

# -ffreestanding ?
# Couldn't get it to "work" (define __STDC_HOSTED__ as 0)

CXXFLAGS="-std=c++17 -g -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector -Wa,--noexecstack"  # -s
LDFLAGS="-L$VENDOR_DIR/GLEW/lib -lGLEW -lGL -lX11 -msse4.1"
INCLUDES="-I$VENDOR_DIR/GLEW/include -I$VENDOR_DIR/GLFW/include -I$VENDOR_DIR/imgui -I$VENDOR_DIR/sci.h -I$VENDOR_DIR/stb -I$VENDOR_DIR/rnd -I$VENDOR_DIR/pt_math"

mkdir -p $BUILD_DIR

g++ $CXXFLAGS $INCLUDES $SRC_DIR/linux_syscall.s $SRC_DIR/linux_platform.s $SRC_DIR/linux_platform.cpp -o $BUILD_DIR/stella $LDFLAGS
STATUS=$?

ctime -end stella_linux.ctm $STATUS
exit $STATUS
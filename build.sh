#!/bin/bash

ctime -begin stella_linux.ctm

BUILD_MODE=static
# BUILD_MODE=dynamic

SRC_DIR=src
BUILD_DIR=build
VENDOR_DIR=vendor

EXECUTABLE=stella

# -ffreestanding ?
# Couldn't get it to "work" (define __STDC_HOSTED__ as 0)

# TODO (IMPORTANT): Call vendor/imgui/build_static.sh (if needed?)

CXXFLAGS="-std=c++17 -g -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector -Wa,--noexecstack"  # -s
LDFLAGS="-L$VENDOR_DIR/GLEW/lib -L$VENDOR_DIR/imgui/lib -l:imgui.a -lGLEW -lGL -lX11 -lgcc -msse4.1"
INCLUDES="-I$VENDOR_DIR/GLEW/include -I$VENDOR_DIR/GLFW/include -I$VENDOR_DIR/imgui -I$VENDOR_DIR/sci.h -I$VENDOR_DIR/stb -I$VENDOR_DIR/rnd -I$VENDOR_DIR/pt_math"

mkdir -p $BUILD_DIR

if [ "$BUILD_MODE" = "static" ]; then
	g++ $CXXFLAGS $INCLUDES $SRC_DIR/linux_syscall.s $SRC_DIR/linux_platform.s $SRC_DIR/linux_platform.cpp $SRC_DIR/stella.cpp -o $BUILD_DIR/stella $LDFLAGS
	STATUS=$?
elif [ "$BUILD_MODE" = "dynamic" ]; then
	echo "dynamic build not yet implemented!"
	STATUS=1
else
	echo "Invalid build mode: $BUILD_MODE"
	STATUS=1
fi

ctime -end stella_linux.ctm $STATUS
exit $STATUS
#!/bin/bash

ctime -begin stella_linux.ctm

SRC_DIR=src
BUILD_DIR=build
VENDOR_DIR=vendor

EXECUTABLE=stella

CXXFLAGS="-std=c++17 -g -DGLEW_NO_GLU"
LDFLAGS="-L$VENDOR_DIR/GLEW/lib -L$VENDOR_DIR/GLFW/lib -L$VENDOR_DIR/imgui/lib -lGLEW -lglfw3 -l:imgui.a -lGL -ldl -lm -lX11 -lpthread"
INCLUDES="-I$VENDOR_DIR/GLEW/include -I$VENDOR_DIR/GLFW/include -I$VENDOR_DIR/imgui -I$VENDOR_DIR/sci.h -I$VENDOR_DIR/stb -I$VENDOR_DIR/rnd"

mkdir -p $BUILD_DIR

g++ $CXXFLAGS $INCLUDES -c -o $BUILD_DIR/main.o $SRC_DIR/main.cpp
# TODO: figure out the bash way of doing this in a less "white-trash" way
if [  $? -ne 0 ]; then
	ctime -end stella_linux.ctm 1
	exit 1
fi
g++ -o $BUILD_DIR/stella $BUILD_DIR/main.o $LDFLAGS
if [ $? -ne 0 ]; then
	ctime -end stella_linux.ctm 1
	exit 1
fi

ctime -end stella_linux.ctm
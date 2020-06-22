#/bin/bash

# TODO: We need the following flag for winderps:
# -mwindows
# ... but, yknow, I don't feel like copying this entire file
# and making build_static_win32.sh or whatever, rn..

BASE_DIR=$(pwd)
VENDOR_DIR=${BASE_DIR}/..
SRC_DIR=${BASE_DIR}/imgui
BUILD_DIR=${BASE_DIR}/lib

DEFINES="-DIMGUI_USER_CONFIG=\"stella_imconfig.hpp\""
CXXFLAGS="-fPIC -std=c++17 -g -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-stack-protector" # -Wa,--noexecstack -fno-unwind-tables -fno-asynchronous-unwind-tables

INCLUDES="-I ${VENDOR_DIR}/stb -I ${VENDOR_DIR}/pt_math -I ${VENDOR_DIR}/../src"
SOURCES="${SRC_DIR}/imgui.cpp ${SRC_DIR}/imgui_draw.cpp ${SRC_DIR}/imgui_widgets.cpp"
OBJECTS="imgui.o imgui_draw.o imgui_widgets.o"

if [ ! -n "$CC" ]; then
CC=g++
fi

mkdir -p $BUILD_DIR
rm -f $BUILD_DIR/*

# remove the pushd/popd
pushd $BUILD_DIR
$CC $CXXFLAGS $DEFINES $INCLUDES -c $SOURCES
ar rvs imgui.a $OBJECTS
popd
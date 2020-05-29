#/bin/bash

BASE_DIR=$(pwd)
VENDOR_DIR=${BASE_DIR}/..
SRC_DIR=${BASE_DIR}/imgui
BUILD_DIR=${BASE_DIR}/lib

CXXFLAGS="-std=c++17 -g -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector -Wa,--noexecstack -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DGLEW_NO_GLU -DSTELLA_IMGUI_STATIC_BUILD -DIMGUI_USER_CONFIG=\"stella_imconfig.hpp\""
LDFLAGS=""

INCLUDES="-I ${VENDOR_DIR}/GLEW/include -I ${VENDOR_DIR}/../src"
SOURCES="${SRC_DIR}/imgui.cpp ${SRC_DIR}/imgui_draw.cpp ${SRC_DIR}/imgui_widgets.cpp"
OBJECTS="imgui.o imgui_draw.o imgui_widgets.o"

mkdir -p lib

# remove the pushd/popd
pushd lib
g++ $CXXFLAGS $INCLUDES -c $SOURCES
ar rvs imgui.a $OBJECTS
popd
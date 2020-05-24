#/bin/bash

BASE_DIR=$(pwd)
VENDOR_DIR=${BASE_DIR}/..
SRC_DIR=${BASE_DIR}/imgui
BUILD_DIR=${BASE_DIR}/lib

CXXFLAGS="-g -std=c++17 -fno-rtti -fno-exceptions -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DGLEW_NO_GLU"
LDFLAGS=""

INCLUDES="-I ${VENDOR_DIR}/GLFW/include -I ${VENDOR_DIR}/GLEW/include"
SOURCES="${SRC_DIR}/imgui.cpp ${SRC_DIR}/imgui_draw.cpp ${SRC_DIR}/imgui_impl_glfw.cpp ${SRC_DIR}/imgui_impl_opengl3.cpp ${SRC_DIR}/imgui_widgets.cpp"
OBJECTS="imgui.o imgui_draw.o imgui_impl_glfw.o imgui_impl_opengl3.o imgui_widgets.o"

mkdir -p lib

# remove the pushd/popd
pushd lib
g++ $CXXFLAGS $INCLUDES -c $SOURCES
ar rvs imgui.a $OBJECTS
popd
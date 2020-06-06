#!/bin/bash

STATUS=0

ctime -begin stella_linux.ctm

# BUILD_MODE=static
BUILD_MODE=dynamic

SRC_DIR=src
BUILD_DIR=build
VENDOR_DIR=vendor

EXECUTABLE=stella
DYLIB=stella.so

# -ffreestanding ?
# Couldn't get it to "work" (define __STDC_HOSTED__ as 0)

# TODO (IMPORTANT): Call vendor/imgui/build_static.sh (if needed?)
# pushd $VENDOR_DIR/imgui
# ./build_static.sh
# STATUS=$?
# popd

if [ "$STATUS" -eq 0 ]; then
CXXFLAGS="-std=c++17 -g -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector -Wa,--noexecstack"  # -s
LDFLAGS="-lgcc -msse4.1"
PLATFORM_LDFLAGS="-lGL -lX11"
GAME_LDFLAGS="-L$VENDOR_DIR/imgui/lib -l:imgui.a"

INCLUDES="-I$SRC_DIR -I$VENDOR_DIR/GLFW/include -I$VENDOR_DIR/imgui -I$VENDOR_DIR/sci.h -I$VENDOR_DIR/stb -I$VENDOR_DIR/rnd -I$VENDOR_DIR/pt_math -I$VENDOR_DIR/GL"
PLATFORM_SOURCES="$SRC_DIR/linux64_syscall.s $SRC_DIR/linux64_platform.s $SRC_DIR/linux64_platform.cpp"
GAME_SOURCES="$SRC_DIR/stella.cpp"

[ -d $BUILD_DIR ] || mkdir -p $BUILD_DIR

if [ "$BUILD_MODE" = "static" ]; then
	g++ $CXXFLAGS $INCLUDES $PLATFORM_SOURCES $GAME_SOURCES -DSTELLA_STATIC -o $BUILD_DIR/$EXECUTABLE $LDFLAGS $PLATFORM_LDFLAGS $GAME_LDFLAGS
	STATUS=$?
elif [ "$BUILD_MODE" = "dynamic" ]; then
	g++ -shared -fPIC $CXXFLAGS $INCLUDES $GAME_SOURCES -DSTELLA_DYNAMIC -o $BUILD_DIR/build_$DYLIB $LDFLAGS $GAME_LDFLAGS
	STATUS=$?

	if [ "$STATUS" -eq 0 ]; then
		[ -f $BUILD_DIR/$DYLIB ] && rm $BUILD_DIR/$DYLIB
		mv $BUILD_DIR/build_$DYLIB $BUILD_DIR/$DYLIB
	fi

	if [ "$STATUS" -eq 0 ]; then
		g++ $CXXFLAGS $INCLUDES $PLATFORM_SOURCES -DSTELLA_DYNAMIC -o $BUILD_DIR/$EXECUTABLE $LDFLAGS $PLATFORM_LDFLAGS -ldl
		STATUS=$?
	fi
else
	echo "Invalid build mode: $BUILD_MODE"
	STATUS=1
fi
fi

ctime -end stella_linux.ctm $STATUS
exit $STATUS
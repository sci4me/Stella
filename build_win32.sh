#!/bin/bash

STATUS=0

ctime -begin stella_linux.ctm

BUILD_MODE=static
# BUILD_MODE=dynamic

SRC_DIR=src
BUILD_DIR=build
VENDOR_DIR=vendor

EXECUTABLE=stella
DYLIB=stella.so

# -ffreestanding ?
# Couldn't get it to "work" (define __STDC_HOSTED__ as 0)

# TODO (IMPORTANT): Call vendor/imgui/build_static.sh (if needed?)
# TODO: make sure we pass $CC correctly
# pushd $VENDOR_DIR/imgui
# ./build_static.sh
# STATUS=$?
# popd

if [ "$STATUS" -eq 0 ]; then
DEFINES="-DSTBI_NO_THREAD_LOCALS"
CXXFLAGS="-std=c++17 -gstabs -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-stack-protector -m64 -mwindows"  # -s -Wa,--noexecstack -fno-unwind-tables -fno-asynchronous-unwind-tables
LDFLAGS="-lgcc -msse4.1"
PLATFORM_LDFLAGS="-lopengl32 -lgdi32 -luser32 -lshell32 -lkernel32 -Wl,-ewin32_main"
GAME_LDFLAGS="-L$VENDOR_DIR/imgui/lib -l:imgui.a"

INCLUDES="-I$SRC_DIR -I$VENDOR_DIR/imgui -I$VENDOR_DIR/stb -I$VENDOR_DIR/rnd -I$VENDOR_DIR/pt_math -I$VENDOR_DIR/GL"
PLATFORM_SOURCES="$SRC_DIR/win32_platform.cpp"
GAME_SOURCES="$SRC_DIR/stella.cpp"

if [ ! -n "$CC" ]; then
CC=g++
fi

[ -d $BUILD_DIR ] || mkdir -p $BUILD_DIR

if [ "$BUILD_MODE" = "static" ]; then
	$CC $CXXFLAGS $DEFINES $INCLUDES $PLATFORM_SOURCES $GAME_SOURCES -DSTELLA_STATIC -o $BUILD_DIR/$EXECUTABLE $LDFLAGS $GAME_LDFLAGS $PLATFORM_LDFLAGS
	STATUS=$?
elif [ "$BUILD_MODE" = "dynamic" ]; then
	$CC -shared -fPIC $CXXFLAGS $DEFINES $INCLUDES $GAME_SOURCES -DSTELLA_DYNAMIC -o $BUILD_DIR/build_$DYLIB $LDFLAGS $GAME_LDFLAGS
	STATUS=$?

	if [ "$STATUS" -eq 0 ]; then
		[ -f $BUILD_DIR/$DYLIB ] && rm $BUILD_DIR/$DYLIB
		mv $BUILD_DIR/build_$DYLIB $BUILD_DIR/$DYLIB

		$CC $CXXFLAGS $INCLUDES $DEFINES $PLATFORM_SOURCES -DSTELLA_DYNAMIC -o $BUILD_DIR/$EXECUTABLE $LDFLAGS $PLATFORM_LDFLAGS -ldl
		STATUS=$?
	fi
else
	echo "Invalid build mode: $BUILD_MODE"
	STATUS=1
fi
fi

ctime -end stella_linux.ctm $STATUS
exit $STATUS

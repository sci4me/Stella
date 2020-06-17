@echo off

set BUILD_MODE=static
rem set BUILD_MODE=dynamic

set SRC_DIR=src
set BUILD_DIR=build
set VENDOR_DIR=vendor

set EXECUTABLE=stella
set DYLIB=stella.so

set CXXFLAGS=-std=c++17 -g -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector
set DEFINES=-DSTELLA_STATIC -DSTBI_NO_THREAD_LOCALS
set LDFLAGS=-msse4.1
set PLATFORM_LDFLAGS=-lopengl32 -luser32 -lkernel32 -lgcc -Wl,-ewin32_main
set GAME_LDFLAGS=-L%VENDOR_DIR%/imgui/lib -l:imgui.a
set INCLUDES=-I%SRC_DIR% -I%VENDOR_DIR%/imgui -I%VENDOR_DIR%/stb -I%VENDOR_DIR%/rnd -I%VENDOR_DIR%/pt_math -I%VENDOR_DIR%/GL
set PLATFORM_SOURCES=%SRC_DIR%/win32_platform.cpp
rem set GAME_SOURCES=%SRC_DIR%/stella.cpp

g++ %CXXFLAGS% %INCLUDES% %PLATFORM_SOURCES% %GAME_SOURCES% %DEFINES% -mwindows -o %BUILD_DIR%/%EXECUTABLE% %GAME_LDFLAGS% %LDFLAGS% %PLATFORM_LDFLAGS%
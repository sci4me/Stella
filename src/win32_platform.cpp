#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#include "platform_interface.hpp"
#include "mylibc.cpp"
#include "win32_mylibc.cpp"

#include <Windows.h>
#include <gl/GL.h>

/*int WINAPI WinMain(HINSTANCE app_instance, HINSTANCE, PSTR args, int show_type) {
    MessageBoxA(nullptr, "", "", MB_OK);
    return 0;
}*/

extern "C" void WinMainCRTStartup() {
    MessageBoxA(nullptr, "", "", MB_OK);
}
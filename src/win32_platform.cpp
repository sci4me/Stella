#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"


#include "platform_interface.hpp"
#include "mylibc.cpp"
#include "win32_mylibc.cpp"


// NOTE:
//  - DllMainCRTStartup will be needed(?) for dynamic mode


// NOTE: In addition to the line above, we'll have to
// create an implement for memset and probably something
// else too; I forget.


#include <windows.h>

extern "C" void win32_main() {
    MessageBoxA(NULL, "fuck", "win32", MB_OK);
}
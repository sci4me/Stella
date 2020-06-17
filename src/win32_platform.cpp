#include <windows.h>
#include <gl/gl.h>

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"


// #define OPENGL_NO_DEFS
#include "platform_interface.hpp"
#include "mylibc.cpp"
#include "win32_mylibc.cpp"


// NOTE:
//  - DllMainCRTStartup will be needed(?) for dynamic mode


// NOTE: In addition to the line above, we'll have to
// create an implement for memset and probably something
// else too; I forget.


LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch(message) {
        case WM_CREATE:
            break;
        case WM_CLOSE:
            PostQuitMessage(0);
    }
    return DefWindowProc(window, message, wparam, lparam);
}


extern "C" void __stdcall win32_main() {
    auto instance = GetModuleHandle(0);


    WNDCLASSA window_class = {};
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = Win32MainWindowCallback;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    // window_class.hIcon = ...;
    window_class.lpszClassName = "StellaWindowClass";
    assert(RegisterClassA(&window_class)); // TODO

    HWND window = CreateWindowExA(
        0,
        window_class.lpszClassName,
        APP_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        instance,
        0
    );
    assert(window); // TODO


    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    MSG msg;
    while(GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        switch(msg.message) {
        }
    }

    mlc_exit(0);
}
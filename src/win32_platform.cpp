#include <windows.h>
#include <gl/gl.h>

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"


// #define OPENGL_NO_DEFS
#include "platform_interface.hpp"
#include "mylibc.cpp"
#include "win32_mylibc.cpp"


typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);



// NOTE:
//  - DllMainCRTStartup will be needed(?) for dynamic mode


// NOTE: In addition to the line above, we'll have to
// create an implement for memset and probably something
// else too; I forget.


volatile static bool running = true;


LRESULT CALLBACK window_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch(msg) {
        case WM_CLOSE: {
            running = false;
            return 0;
        }
    }

    return DefWindowProcA(window, msg, wparam, lparam);
}


extern "C" void __stdcall win32_main() {
    auto instance = GetModuleHandle(0);


    WNDCLASSA window_class = {};
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = window_callback;
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

    HDC dc = GetDC(window);



    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    MSG msg;
    while(running) {
        MSG msg;
        while(GetMessage(&msg, 0, 0, 0) && running) {
            switch(msg.message) {
                case WM_QUIT: {
                    running = false;
                    break;
                }
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP: {
                    break;
                }
                default: {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    break;
                }
            }
        }


    }

    mlc_exit(0);
}
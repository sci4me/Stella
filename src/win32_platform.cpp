#include <windows.h>
#include <GL/gl.h>
#include <GL/wglext.h>

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"


// #define OPENGL_NO_DEFS
#include "platform_interface.hpp"
#include "mylibc.cpp"
#include "win32_mylibc.cpp"


#if defined(STELLA_DYNAMIC)
#include <dlfcn.h>
Game_Attach *stella_attach;
Game_Init *stella_init;
Game_Deinit *stella_deinit;
Game_Update_And_Render *stella_update_and_render;
#elif defined(STELLA_STATIC)
extern "C" GAME_INIT(stella_init);
extern "C" GAME_DEINIT(stella_deinit);
extern "C" GAME_UPDATE_AND_RENDER(stella_update_and_render);
#endif


typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext, const int *attribList);

wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;
wgl_create_context_attribs_arb *wglCreateContextAttribsARB;


// NOTE:
//  - DllMainCRTStartup will be needed(?) for dynamic mode


// NOTE: In addition to the line above, we'll have to
// create an implement for memset and probably something
// else too; I forget.


void set_pixel_format(HDC dc) {
    s32 suggested_pixel_format_index;
    u32 extended_pick;

    if(wglChoosePixelFormatARB) {
        s32 attribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            0,
        };
        
        s32 index;
        wglChoosePixelFormatARB(dc, attribs, 0, 1, &suggested_pixel_format_index, &extended_pick);
    }

    if(!extended_pick) {
        PIXELFORMATDESCRIPTOR pixel_format_desc;
        pixel_format_desc.nSize = sizeof(pixel_format_desc);
        pixel_format_desc.nVersion = 1;
        pixel_format_desc.iPixelType = PFD_TYPE_RGBA;
        pixel_format_desc.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pixel_format_desc.cColorBits = 32;
        pixel_format_desc.cAlphaBits = 8;
        pixel_format_desc.cDepthBits = 24;
        pixel_format_desc.iLayerType = PFD_MAIN_PLANE;
        suggested_pixel_format_index = ChoosePixelFormat(dc, &pixel_format_desc);
    }

    PIXELFORMATDESCRIPTOR suggested_pixel_format;
    DescribePixelFormat(dc, suggested_pixel_format_index, sizeof(suggested_pixel_format), &suggested_pixel_format);
    SetPixelFormat(dc, suggested_pixel_format_index, &suggested_pixel_format);
}

bool load_wgl_functions() {
    WNDCLASSA clss = {};
    clss.lpfnWndProc = DefWindowProcA;
    clss.hInstance = GetModuleHandle(0);
    clss.lpszClassName = "StellaWGLLoader";
    assert(RegisterClassA(&clss));

    HWND window = CreateWindowExA(
        0,
        clss.lpszClassName,
        "Stella",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        clss.hInstance,
        0
    );
    assert(window); // TODO

    HDC dc = GetDC(window);

    set_pixel_format(dc);

    bool result = true;

    HGLRC glrc = wglCreateContext(dc);
    if(wglMakeCurrent(dc, glrc)) {
        wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb*) wglGetProcAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (wgl_create_context_attribs_arb*) wglGetProcAddress("wglCreateContextAttribsARB");
        result = wglChoosePixelFormatARB != 0 &&
                 wglCreateContextAttribsARB != 0;
    }

    wglMakeCurrent(0, 0);
    wglDeleteContext(glrc);
    ReleaseDC(window, dc);
    DestroyWindow(window);

    return result;
}


volatile static bool running = true;


void load_opengl(OpenGL *gl) {
    // NOTE TODO: This is silly :P But hey, yknow. *shrugs*
    // Eventually we may want/need to actually do extension
    // checking, etc. etc. etc.
    // Although I kind of feel like, at least for now,
    // just having a list of required GL functions and no
    // optional ones is fine; I'd like to avoid having optional
    // ones for as long as we can get away with it.
    //              - sci4me, 6/4/20 (6/17/20)

    HMODULE lib = LoadLibraryA("opengl32.dll");

    #define PACK(name, ret, params) gl->name = (gl##name##_fn*) wglGetProcAddress("gl" #name); if(!gl->name) { gl->name = (gl##name##_fn*) GetProcAddress(lib, "gl" #name); } assert(gl->name != nullptr);
    OPENGL_FUNCTIONS(PACK)
    #undef PACK

    FreeLibrary(lib); // TODO: is this safe?
}


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

    assert(load_wgl_functions());
    set_pixel_format(dc);

    s32 attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, GL_MAJOR,
        WGL_CONTEXT_MINOR_VERSION_ARB, GL_MINOR,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
    #ifdef DEBUG
            | WGL_CONTEXT_DEBUG_BIT_ARB
    #endif
        ,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC glrc = wglCreateContextAttribsARB(dc, 0, attribs);
    assert(glrc);

    assert(wglMakeCurrent(dc, glrc));


    GLint gl_major, gl_minor; 
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major); 
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    // NOTE We assert here because it should be impossible to get 
    // this far if we weren't able to get a GL context at or 
    // above the requested version.
    assert(gl_major >= GL_MAJOR && gl_minor >= GL_MINOR);


    PlatformIO pio = {};

    load_opengl(&pio.gl);

    
    #ifdef STELLA_DYNAMIC
    // TODO
    #endif

    stella_init(&pio);


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
                case WM_PAINT: {
                    stella_update_and_render(&pio);
                    SwapBuffers(dc);
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
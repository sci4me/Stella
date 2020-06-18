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
    s32 suggested_pixel_format_index = 0;
    u32 extended_pick = 0;

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
    assert(DescribePixelFormat(dc, suggested_pixel_format_index, sizeof(suggested_pixel_format), &suggested_pixel_format));
    assert(SetPixelFormat(dc, suggested_pixel_format_index, &suggested_pixel_format));
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
    } else {
        result = false;
    }

    wglMakeCurrent(0, 0);
    wglDeleteContext(glrc);
    ReleaseDC(window, dc);
    DestroyWindow(window);

    return result;
}


volatile static bool running = true;
volatile static bool resized = false;


static void load_opengl(OpenGL *gl) {
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


static Virtual_Button key_map[256] = {};

static void init_key_map() {
    mlc_memset(key_map, VB_INVALID, sizeof(key_map));

    key_map[VK_ESCAPE] = VB_ESC;
    key_map['A'] = VB_A;
    key_map['B'] = VB_B;
    key_map['C'] = VB_C;
    key_map['D'] = VB_D;
    key_map['E'] = VB_E;
    key_map['F'] = VB_F;
    key_map['G'] = VB_G;
    key_map['H'] = VB_H;
    key_map['I'] = GK_I;
    key_map['J'] = VB_J;
    key_map['K'] = VB_K;
    key_map['L'] = VB_L;
    key_map['M'] = VB_M;
    key_map['N'] = VB_N;
    key_map['O'] = VB_O;
    key_map['P'] = VB_P;
    key_map['Q'] = VB_Q;
    key_map['R'] = VB_R;
    key_map['S'] = VB_S;
    key_map['T'] = VB_T;
    key_map['U'] = VB_U;
    key_map['V'] = VB_V;
    key_map['W'] = VB_W;
    key_map['X'] = VB_X;
    key_map['Y'] = VB_Y;
    key_map['Z'] = VB_Z;
    key_map['0'] = VB_0;
    key_map['1'] = VB_1;
    key_map['2'] = VB_2;
    key_map['3'] = VB_3;
    key_map['4'] = VB_4;
    key_map['5'] = VB_5;
    key_map['6'] = VB_6;
    key_map['7'] = VB_7;
    key_map['8'] = VB_8;
    key_map['9'] = VB_9;
    key_map[VK_F1] = VB_F1;
    key_map[VK_F2] = VB_F2;
    key_map[VK_F3] = VB_F3;
    key_map[VK_F4] = VB_F4;
    key_map[VK_F5] = VB_F5;
    key_map[VK_F6] = VB_F6;
    key_map[VK_F7] = VB_F7;
    key_map[VK_F8] = VB_F8;
    key_map[VK_F9] = VB_F9;
    key_map[VK_F10] = VB_F10;
    key_map[VK_F11] = VB_F11;
    key_map[VK_F12] = VB_F12;
    key_map[VK_LEFT] = VB_LEFT;
    key_map[VK_RIGHT] = VB_RIGHT;
    key_map[VK_UP] = VB_UP;
    key_map[VK_DOWN] = VB_DOWN;
    key_map[VK_PRIOR] = VB_PAGE_UP;
    key_map[VK_NEXT] = VB_PAGE_DOWN;
    key_map[VK_INSERT] = VB_INSERT;
    key_map[VK_DELETE] = VB_DELETE;
    key_map[VK_HOME] = VB_HOME;
    key_map[VK_END] = VB_END;
    key_map[VK_BACK] = VB_BACKSPACE;
    key_map[VK_RETURN] = VB_ENTER;
    // key_map[KC_KP_ENTER] = VB_KP_ENTER;
    key_map[VK_TAB] = VB_TAB;
    key_map[VK_SPACE] = VB_SPACE;
    //key_map[KC_CTRL_LEFT] = VB_CTRL_LEFT;
    //key_map[KC_CTRL_RIGHT] = VB_CTRL_RIGHT;
    //key_map[KC_SHIFT_LEFT] = VB_SHIFT_LEFT;
    //key_map[KC_SHIFT_RIGHT] = VB_SHIFT_RIGHT;
    //key_map[KC_ALT_LEFT] = VB_ALT_LEFT;
    //key_map[KC_ALT_RIGHT] = VB_ALT_RIGHT;
    //key_map[KC_SUPER_LEFT] = VB_SUPER_LEFT;
    //key_map[KC_SUPER_RIGHT] = VB_SUPER_RIGHT;
}


static void update_button_state(PlatformIO *pio, Virtual_Button vb, bool state) {
    pio->button_state[vb] = BTN_FLAG_NONE;
    if(state) {
        pio->button_state[vb] |= BTN_FLAG_DOWN;
        pio->button_state[vb] |= BTN_FLAG_PRESSED;
    } else {
        pio->button_state[vb] |= BTN_FLAG_RELEASED;
    }
}

LRESULT CALLBACK window_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch(msg) {
        case WM_CLOSE: {
            running = false;
            return 0;
        }
        case WM_SIZE: {
            resized = true;
            break;
        }
    }

    return DefWindowProcA(window, msg, wparam, lparam);
}


static void init_console() {
    assert(AllocConsole());

    HANDLE hConOut = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hConIn = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
    SetStdHandle(STD_ERROR_HANDLE, hConOut);
    SetStdHandle(STD_INPUT_HANDLE, hConIn);
}


int platform_main() {
    init_console();

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


    init_key_map();


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


    // TODO: window focus

    MSG msg;
    while(running) {
        pio.mouse_wheel_x = 0.0f;
        pio.mouse_wheel_y = 0.0f;

        for(u32 i = 0; i < array_length(pio.button_state); i++) {
            pio.button_state[i] &= ~(BTN_FLAG_PRESSED | BTN_FLAG_RELEASED);
        }


        MSG msg;
        while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            switch(msg.message) {
                case WM_QUIT: {
                    running = false;
                    break;
                }
                case WM_SETFOCUS: {
                    // pio.window_focused = true;
                    break;
                }
                case WM_KILLFOCUS: {
                    // pio.window_focused = false;
                    break;
                }
                case WM_LBUTTONDOWN: {
                    update_button_state(&pio, VMB_LEFT, true);
                    break;
                }
                case WM_LBUTTONUP: {
                    update_button_state(&pio, VMB_LEFT, false);
                    break;
                }
                case WM_MBUTTONDOWN: {
                    update_button_state(&pio, VMB_MIDDLE, true);
                    break;
                }
                case WM_MBUTTONUP: {
                    update_button_state(&pio, VMB_MIDDLE, false);
                    break;
                }
                case WM_RBUTTONDOWN: {
                    update_button_state(&pio, VMB_RIGHT, true);
                    break;
                }
                case WM_RBUTTONUP: {
                    update_button_state(&pio, VMB_RIGHT, false);
                    break;
                }
                case WM_MOUSEWHEEL: {
                    s16 x = (s16) HIWORD(msg.wParam);
                    if(x > 0)       pio.mouse_wheel_y += 1;
                    else if(x < 0)  pio.mouse_wheel_y -= 1;
                }
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP: {
                    u32 key = (u32) msg.wParam;

                    bool alt_down = (msg.lParam & (1 << 29));
                    bool shift_down = (GetKeyState(VK_SHIFT) & (1 << 15));

                    bool was_down = ((msg.lParam & (1 << 30)) != 0);
                    bool is_down = ((msg.lParam & (1UL << 31)) == 0);
                    if(was_down != is_down) {
                        if(key >= 0 && key < array_length(key_map)) {
                            auto vk = key_map[key];
                            if(vk != VB_INVALID) update_button_state(&pio, vk, is_down);
                        }
                    }
                    break;
                }
                default: {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    break;
                }
            }
        }


        {
            POINT mp;
            GetCursorPos(&mp);
            ScreenToClient(window, &mp);
            pio.mouse_x = (f32) mp.x;
            pio.mouse_y = (f32) mp.y;
        }


        pio.window_just_resized = resized;
        if(resized) {
            resized = false;
            RECT size;
            GetClientRect(window, &size);
            pio.window_width = (s32)(size.right - size.left);
            pio.window_height = (s32)(size.bottom - size.top);
        }


        pio.delta_time = 1.0f; // TODO

        stella_update_and_render(&pio);


        SwapBuffers(wglGetCurrentDC());
    }

    stella_deinit(&pio);

    // TODO: deinit windows shit

    return 0;
}

extern "C" void __stdcall win32_main() {
    mlc_exit(platform_main());
}
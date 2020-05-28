#include "mylibc.cpp"

#include "platform_interface.hpp"

// TODO: Remove this.. er .. something.
void tprintf(char const* fmt, ...);

// #include "stella.hpp"
// #include "stella.cpp"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>


// TODO: These need to be surrounded by an ifdef (or similar) and
// and else clause for the case when we're doing dynamic loading.
extern "C" GAME_INIT(stella_init);
extern "C" GAME_DEINIT(stella_deinit);
extern "C" GAME_UPDATE_AND_RENDER(stella_update_and_render);


typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);


typedef u8 KeyCode;
enum KeyCode_ : KeyCode {
    KC_ESC              = 9,
    KC_F1               = 67,
    KC_F2               = 68,
    KC_F3               = 69,
    KC_F4               = 70,
    KC_F5               = 71,
    KC_F6               = 72,
    KC_F7               = 73,
    KC_F8               = 74,
    KC_F9               = 75,
    KC_F10              = 76,
    KC_F11              = 95,
    KC_F12              = 96,
    KC_PRINT_SCREEN     = 111,
    KC_SCROLL_LOCK      = 78,
    KC_PAUSE            = 110,
    KC_GRAVE            = 49,
    KC_1                = 10,
    KC_2                = 11,
    KC_3                = 12,
    KC_4                = 13,
    KC_5                = 14,
    KC_6                = 15,
    KC_7                = 16,
    KC_8                = 17,
    KC_9                = 18,
    KC_0                = 19,
    KC_MINUS            = 20,
    KC_EQUALS           = 21,
    KC_BACKSPACE        = 22,
    KC_INSERT           = 106,
    KC_HOME             = 97,
    KC_PAGE_UP          = 99,
    KC_NUM_LOCK         = 77,
    KC_KP_SLASH         = 112,
    KC_KP_ASTERISK      = 63,
    KC_KP_MINUS         = 82,
    KC_TAB              = 23,
    KC_Q                = 23,
    KC_W                = 25,
    KC_E                = 26,
    KC_R                = 27,
    KC_T                = 28,
    KC_Y                = 29,
    KC_U                = 30,
    KC_I                = 31,
    KC_O                = 32,
    KC_P                = 33,
    KC_LBRACK           = 34,
    KC_RBRACK           = 35,
    KC_RETURN           = 36,
    KC_DELETE           = 107,
    KC_END              = 103,
    KC_PAGE_DOWN        = 105,
    KC_KP_7             = 79,
    KC_KP_8             = 80,
    KC_KP_9             = 81,
    KC_KP_PLUS          = 86,
    KC_CAPS_LOCK        = 66,
    KC_A                = 38,
    KC_S                = 39,
    KC_D                = 40,
    KC_F                = 41,
    KC_G                = 42,
    KC_H                = 43,
    KC_J                = 44,
    KC_K                = 45,
    KC_L                = 46,
    KC_SEMICOLON        = 47,
    KC_APOSTROPHE       = 48,
    KC_KP_4             = 83,
    KC_KP_5             = 84,
    KC_SHIFT_LEFT       = 50,
    KC_INTERNATIONAL    = 94,
    KC_Z                = 52,
    KC_X                = 53,
    KC_C                = 54,
    KC_V                = 55,
    KC_B                = 56,
    KC_N                = 57,
    KC_M                = 58,
    KC_COMMA            = 59,
    KC_PERIOD           = 60,
    KC_SLASH            = 61,
    KC_SHIFT_RIGHT      = 62,
    KC_BACKSLASH        = 51,
    KC_CURSOR_UP        = 98,
    KC_KP_1             = 87,
    KC_KP_2             = 88,
    KC_KP_3             = 89,
    KC_KP_ENTER         = 108,
    KC_CTRL_LEFT        = 37,
    KC_LOGO_LEFT        = 115,
    KC_ALT_LEFT         = 64,
    KC_SPACE            = 65,
    KC_ALT_RIGHT        = 113,
    KC_LOGO_RIGHT       = 116,
    KC_MENU             = 117,
    KC_CTRL_RIGHT       = 109,
    KC_CURSOR_LEFT      = 100,
    KC_CURSOR_DOWN      = 104,
    KC_CURSOR_RIGHT     = 102,
    KC_KP_0             = 90,
    KC_KP_PERIOD        = 91
};

typedef u8 MouseButtonCode;
enum MouseButtonCode_ : MouseButtonCode {
    MB_LEFT = 1,
    MB_MIDDLE = 2,
    MB_RIGHT = 3
};


static Virtual_Button key_map[256] = {};

static void init_key_map() {
    mlc_memset(key_map, VB_INVALID, sizeof(key_map));

    key_map[KC_ESC] = VK_ESC;
    key_map[KC_A] = VK_A;
    key_map[KC_B] = VK_B;
    key_map[KC_C] = VK_C;
    key_map[KC_D] = VK_D;
    key_map[KC_E] = VK_E;
    key_map[KC_F] = VK_F;
    key_map[KC_G] = VK_G;
    key_map[KC_H] = VK_H;
    key_map[KC_I] = GK_I;
    key_map[KC_J] = VK_J;
    key_map[KC_K] = VK_K;
    key_map[KC_L] = VK_L;
    key_map[KC_M] = VK_M;
    key_map[KC_N] = VK_N;
    key_map[KC_O] = VK_O;
    key_map[KC_P] = VK_P;
    key_map[KC_Q] = VK_Q;
    key_map[KC_R] = VK_R;
    key_map[KC_S] = VK_S;
    key_map[KC_T] = VK_T;
    key_map[KC_U] = VK_U;
    key_map[KC_V] = VK_V;
    key_map[KC_W] = VK_W;
    key_map[KC_X] = VK_X;
    key_map[KC_Y] = VK_Y;
    key_map[KC_Z] = VK_Z;
    key_map[KC_0] = VK_0;
    key_map[KC_1] = VK_1;
    key_map[KC_2] = VK_2;
    key_map[KC_3] = VK_3;
    key_map[KC_4] = VK_4;
    key_map[KC_5] = VK_5;
    key_map[KC_6] = VK_6;
    key_map[KC_7] = VK_7;
    key_map[KC_8] = VK_8;
    key_map[KC_9] = VK_9;
    key_map[KC_F1] = VK_F1;
    key_map[KC_F2] = VK_F2;
    key_map[KC_F3] = VK_F3;
    key_map[KC_F4] = VK_F4;
    key_map[KC_F5] = VK_F5;
    key_map[KC_F6] = VK_F6;
    key_map[KC_F7] = VK_F7;
    key_map[KC_F8] = VK_F8;
    key_map[KC_F9] = VK_F9;
    key_map[KC_F10] = VK_F10;
    key_map[KC_F11] = VK_F11;
    key_map[KC_F12] = VK_F12;
}


// NOTE: This is hacky but meh, it's what we've got.
bool x_error_occurred = false;
static int x_error_handler(Display *dsp, XErrorEvent *ev) {
    x_error_occurred = true;
    return 0;
}


enum {
    _NET_WM_STATE_REMOVE    = 0,
    _NET_WM_STATE_ADD       = 1,
    _NET_WM_STATE_TOGGLE    = 2
};

static void set_fullscreen(Display *display, Window window, bool fullscreen) {
    XEvent event = {};

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    // event.xclient.display = display;
    event.xclient.window = window;
    event.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", False);
    event.xclient.format = 32;
    event.xclient.data.l[0] = (fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE);
    event.xclient.data.l[1] = (long) XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    event.xclient.data.l[2] = 0;
    
    XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask | SubstructureRedirectMask, &event);
    XFlush(display);
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


s32 main(s32 argc, char **argv) {
	Display *dsp = XOpenDisplay(0);
    if(!dsp) {
    	tprintf("Failed to open X display!\n");
        return 1;
    }

    GLint glx_major, glx_minor;
    glXQueryVersion(dsp, &glx_major, &glx_minor);
    if(glx_major < 1 || glx_minor < 4) { // NOTE TODO: Don't hardcode the desired GLX version
    	tprintf("GLX version too old! Got %d.%d, want 1.4+\n", glx_major, glx_minor);
        XCloseDisplay(dsp);
        return 1;
    }

    Window root = DefaultRootWindow(dsp);

    GLint att[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };

    int fb_count;
    GLXFBConfig *fbcs = glXChooseFBConfig(dsp, DefaultScreen(dsp), att, &fb_count);
    if(fbcs == 0) {
    	tprintf("Failed to choose framebuffer config!\n");
        XCloseDisplay(dsp);
        return 1;
    }

    // TODO: Pick the best out of fbcs
    XVisualInfo *vi = glXGetVisualFromFBConfig(dsp, fbcs[0]);
    if(!vi) {
    	tprintf("Failed to get XVisualInfo from framebuffer config!\n");
        XCloseDisplay(dsp);
        return 1;
    }

    Colormap cmap = XCreateColormap(dsp, root, vi->visual, AllocNone);

    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

    Window win = XCreateWindow(dsp, root, 0, 0, 1280, 720, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    if(!win) {
    	tprintf("Failed to create X window!\n");
        XCloseDisplay(dsp);
        return 1;
    }
    
    XFree(vi);

    XStoreName(dsp, win, APP_NAME);

    auto glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddress((GLubyte const*)"glXCreateContextAttribsARB");
    if(!glXCreateContextAttribsARB) {
        tprintf("glXCreateContextAttribsARB is not available!\n");
        XDestroyWindow(dsp, win);
        XCloseDisplay(dsp);
        return 1;
    }

    int ctx_att[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, GL_MAJOR,
        GLX_CONTEXT_MINOR_VERSION_ARB, GL_MINOR,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        None
    };

    auto eh = XSetErrorHandler(x_error_handler);
    GLXContext glc = glXCreateContextAttribsARB(dsp, fbcs[0], 0, True, ctx_att);
    XSetErrorHandler(eh);

    if(x_error_occurred) {
        tprintf("Failed to create GLX context!\n");
        XDestroyWindow(dsp, win);
        XCloseDisplay(dsp);
        return 1;
    }

    XSync(dsp, False);

    Atom atomWmDeleteWindow = XInternAtom(dsp, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dsp, win, &atomWmDeleteWindow, 1);

    if(!glXIsDirect(dsp, glc)) {
    	tprintf("GLX gave us an indirect GL context even though we asked for a direct one! :(\n");
        glXDestroyContext(dsp, glc);
        XDestroyWindow(dsp, win);
        XCloseDisplay(dsp);
        return 1;
    }


    glXMakeCurrent(dsp, win, glc);


    if(glewInit() != GLEW_OK) {
        tprintf("Failed to initialize GLEW!\n");

        glXMakeCurrent(dsp, None, 0);
        glXDestroyContext(dsp, glc);
        XDestroyWindow(dsp, win);
        XCloseDisplay(dsp);
        return 1;
    }


    GLint gl_major, gl_minor; 
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major); 
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    // NOTE We assert here because it should be impossible to get 
    // this far if we weren't able to get a GL context at or 
    // above the requested version.
    assert(gl_major >= GL_MAJOR && gl_minor >= GL_MINOR);


    init_key_map();


    XMapRaised(dsp, win);


    PlatformIO pio = {};

    stella_init(&pio);


    bool fullscreen = false;
    bool running = true;
    while(running) {
        // NOTE: Reset the necessary input state so 
        // it can be reset (or not) by the loop below.
        pio.mouse_wheel_x = 0.0f;
        pio.mouse_wheel_y = 0.0f;

        for(u32 i = 0; i < array_length(pio.button_state); i++) {
            pio.button_state[i] &= ~(BTN_FLAG_PRESSED | BTN_FLAG_RELEASED);
        }

        while(XPending(dsp)) {
            XEvent xev;
            XNextEvent(dsp, &xev);

            switch(xev.type) {
                case ConfigureNotify: {
                    if(xev.xconfigure.width != pio.window_width || xev.xconfigure.height != pio.window_height) {
                        pio.window_width = xev.xconfigure.width;
                        pio.window_height = xev.xconfigure.height;
                        pio.window_just_resized = true;
                    }
                    break;
                }
                case ClientMessage: {
                    if(xev.xclient.data.l[0] == atomWmDeleteWindow) {
                        running = false;
                    }
                    break;
                }
                case KeyPress:
                case KeyRelease: {
                    // NOTE: Skip key repeat events.
                    if(xev.type == KeyRelease && XEventsQueued(dsp, QueuedAfterReading)) {
                        XEvent next;
                        XPeekEvent(dsp, &next);
                        if ((next.type == KeyPress) && (next.xkey.time == xev.xkey.time) && (next.xkey.keycode == xev.xkey.keycode)) {
                            XNextEvent(dsp, &xev);
                            continue;
                        }
                    }

                    if(xev.type == KeyPress && xev.xkey.keycode == KC_F11) {
                        fullscreen = !fullscreen;
                        
                        // NOTE: We will process the size change event generated by this
                        // before calling update_and_render since we're processing events
                        // in a loop.
                        set_fullscreen(dsp, win, fullscreen);
                    } else {
                        if(xev.xkey.keycode >= 0 && xev.xkey.keycode < array_length(key_map)) {
                            auto vk = key_map[xev.xkey.keycode];
                            if(vk != VB_INVALID) update_button_state(&pio, vk, xev.type == KeyPress);
                        }
                    }
                    break;
                }
                case ButtonPress:
                case ButtonRelease: {
                    // TODO: horizontal scroll and also, 1?
                    if(xev.xbutton.button == Button4 && xev.type == ButtonPress) {
                        pio.mouse_wheel_y = 1.0f;
                    } else if(xev.xbutton.button == Button5 && xev.type == ButtonPress) {
                        pio.mouse_wheel_y = -1.0f;
                    } else {
                        bool state = xev.type == ButtonPress;
                        switch(xev.xbutton.button) {
                            case MB_LEFT: update_button_state(&pio, VMB_LEFT, state); break;
                            case MB_MIDDLE: update_button_state(&pio, VMB_MIDDLE, state); break;
                            case MB_RIGHT: update_button_state(&pio, VMB_RIGHT, state); break;
                        }
                    }
                    break;
                }
                case MotionNotify: {
                    break;
                }
            }
        }


        Window root_r, child_r;
        s32 root_x, root_y;
        s32 win_x, win_y;
        u32 mask_r;
        if(XQueryPointer(dsp, win, &root_r, &child_r, &root_x, &root_y, &win_x, &win_y, &mask_r)) {
            if(win_x >= 0 && win_y >= 0 && win_x < pio.window_width && win_y < pio.window_height) {
                pio.mouse_x = (f32) win_x;
                pio.mouse_y = (f32) win_y;
            } else {
                pio.mouse_x = -FLT_MAX;
                pio.mouse_y = -FLT_MAX;
            }
        }


        stella_update_and_render(&pio);


        pio.window_just_resized = false;


        glXSwapBuffers(dsp, win);
    }


    stella_deinit(&pio);


    glXMakeCurrent(dsp, None, 0);
    glXDestroyContext(dsp, glc);
    XDestroyWindow(dsp, win);
    XCloseDisplay(dsp);

	return 0;
}
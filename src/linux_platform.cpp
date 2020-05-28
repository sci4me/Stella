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


// TODO: Audit these? Or something?
// I don't remember typing some of them lol,
// So, I dunno maybe I just copied some of them
// from Handmade Hero (linux edition).
// Actually yeah that must be what happened. Lol
#define KEYCODE_W           25
#define KEYCODE_A           38
#define KEYCODE_S           39
#define KEYCODE_D           40
#define KEYCODE_Q           24
#define KEYCODE_E           26
#define KEYCODE_UP          111
#define KEYCODE_DOWN        116
#define KEYCODE_LEFT        113
#define KEYCODE_RIGHT       114
#define KEYCODE_ESCAPE      9
#define KEYCODE_ENTER       36
#define KEYCODE_SPACE       65
#define KEYCODE_P           33
#define KEYCODE_L           46
#define KEYCODE_C           54
#define KEYCODE_SHIFT_L     50
#define KEYCODE_SHIFT_R     62
#define KEYCODE_CTRL_L      37
#define KEYCODE_CTRL_R      105
#define KEYCODE_ALT_L       64
#define KEYCODE_ALT_R       108
#define KEYCODE_SUPER       133
#define KEYCODE_PLUS        21
#define KEYCODE_MINUS       20
#define KEYCODE_F1          67
#define KEYCODE_F2          68
#define KEYCODE_F3          69
#define KEYCODE_F4          70
#define KEYCODE_F10         76
#define KEYCODE_F11         95
#define KEYCODE_F12         96
#define KEYCODE_ESC         9


// NOTE: This is hacky but meh, it's what we've got.
bool x_error_occurred = false;
int x_error_handler(Display *dsp, XErrorEvent *ev) {
    x_error_occurred = true;
    return 0;
}


enum {
    _NET_WM_STATE_REMOVE    = 0,
    _NET_WM_STATE_ADD       = 1,
    _NET_WM_STATE_TOGGLE    = 2
};

void set_fullscreen(Display *display, Window window, bool fullscreen) {
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


    XMapRaised(dsp, win);


    PlatformIO pio = {};

    stella_init(&pio);


    bool fullscreen = false;
    bool running = true;
    while(running) {
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

                    if(xev.type == KeyPress && xev.xkey.keycode == KEYCODE_F11) {
                        fullscreen = !fullscreen;
                        
                        // NOTE: We will process the size change event generated by this
                        // before calling update_and_render since we're processing events
                        // in a loop.
                        set_fullscreen(dsp, win, fullscreen);
                    } else {
                        // TODO
                        // game.key_callback(xev.xkey.keycode, xev.type == KeyPress);
                    }
                    break;
                }
                case ButtonPress:
                case ButtonRelease: {
                    // TODO: horizontal scroll and also, 1?

                    // TODO
                    if(xev.xbutton.button == Button4 && xev.type == ButtonPress) {
                        // game.scroll_callback(0, 1);
                    } else if(xev.xbutton.button == Button5 && xev.type == ButtonPress) {
                        // game.scroll_callback(0, -1);
                    } else {
                        // game.mouse_button_callback(xev.xbutton.button, xev.type == ButtonPress);
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
            // TODO
            // game.mouse_position_callback(win_x, win_y, win_x >= 0 && win_y >= 0 && win_x < game.window_width && win_y < game.window_height);
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
#include "mylibc.cpp"

#include "linux_platform.hpp"
#include "platform_interface.hpp"

#define GLEW_STATIC
#define GLEW_NO_GLU
#include "GL/glew.h"

// TODO: Remove this.. er .. something.
void tprintf(char const* fmt, ...);

#include "stella.hpp"
#include "stella.cpp"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>


typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);


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


    dump_gl_info();
    // dump_gl_extensions();

    GLint gl_major, gl_minor; 
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major); 
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    // NOTE We assert here because it should be impossible to get 
    // this far if we weren't able to get a GL context at or 
    // above the requested version.
    assert(gl_major >= GL_MAJOR && gl_minor >= GL_MINOR);


    XMapRaised(dsp, win);


    Game game;
    game.init();


    XWindowAttributes gwa;
    XGetWindowAttributes(dsp, win, &gwa);
    game.window_size_callback(gwa.width, gwa.height);


    PlatformIO pio;

    stella_init(&pio);


    bool fullscreen = false;
    bool running = true;
    while(running) {
        while(XPending(dsp)) {
            XEvent xev;
            XNextEvent(dsp, &xev);

            switch(xev.type) {
                case ConfigureNotify: {
                    if(xev.xconfigure.width != game.window_width || xev.xconfigure.height != game.window_height) {
                        game.window_size_callback(xev.xconfigure.width, xev.xconfigure.height);
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
                        game.key_callback(xev.xkey.keycode, xev.type == KeyPress);
                    }
                    break;
                }
                case ButtonPress:
                case ButtonRelease: {
                    // TODO: horizontal scroll and also, 1?

                    if(xev.xbutton.button == Button4 && xev.type == ButtonPress) {
                        game.scroll_callback(0, 1);
                    } else if(xev.xbutton.button == Button5 && xev.type == ButtonPress) {
                        game.scroll_callback(0, -1);
                    } else {
                        game.mouse_button_callback(xev.xbutton.button, xev.type == ButtonPress);
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
            game.mouse_position_callback(win_x, win_y, win_x >= 0 && win_y >= 0 && win_x < game.window_width && win_y < game.window_height);
        }


        game.update_and_render();
	        

        stella_update_and_render(&pio);


        glXSwapBuffers(dsp, win);

        tclear();
    }


    game.deinit();


    glXMakeCurrent(dsp, None, 0);
    glXDestroyContext(dsp, glc);
    XDestroyWindow(dsp, win);
    XCloseDisplay(dsp);

	return 0;
}
#include "linux_platform.hpp"

#define GLEW_STATIC
#define GLEW_NO_GLU
#include "GL/glew.h"

#include "mylibc.cpp"

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


s32 main(s32 argc, char **argv) {
	Display *dsp = XOpenDisplay(0);
    if(!dsp) {
    	tprintf("Failed to open X display!\n");
        return 1;
    }

    GLint glx_major, glx_minor;
    glXQueryVersion(dsp, &glx_major, &glx_minor);
    if(glx_major < 1 || glx_minor < 4) {
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

    Window win = XCreateWindow(dsp, root, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
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

    // TODO X error handler thing...
    
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

                    game.key_callback(xev.xkey.keycode, xev.type == KeyPress);
                    break;
                }
                case ButtonPress:
                case ButtonRelease: {
                    break;
                }
                case MotionNotify: {
                    break;
                }
            }
        }


        game.update_and_render();
	        

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
#include <string.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

#include "common.h"

static const int glxattribs[] = {
    GLX_X_RENDERABLE, True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_CONFIG_CAVEAT, GLX_NONE,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DOUBLEBUFFER, True,
    0
  };

static Display *display;
static Window window;
static GLXDrawable drawable;
static GLXContext context;

void os_open_window() {
  int nglxconfigs = 0;
  GLXFBConfig *glxconfigs = NULL;
  XVisualInfo *vinfo = NULL;
  XSetWindowAttributes winattrs;
  Atom delete_message;

  display = XOpenDisplay(NULL);
  CHECK(display, "XOpenDisplay");

  glxconfigs = glXChooseFBConfig(display, 0, glxattribs, &nglxconfigs);
  CHECK(glxconfigs && nglxconfigs, "glXChooseFBConfig");

  vinfo = glXGetVisualFromFBConfig(display, glxconfigs[0]);
  CHECK(vinfo, "glXGetVisualFromFBConfig");

  memset(&winattrs, 0, sizeof(winattrs));
  winattrs.event_mask =
    ExposureMask | VisibilityChangeMask | StructureNotifyMask |
    KeyPressMask | PointerMotionMask;
  winattrs.border_pixel = 0;
  winattrs.bit_gravity = StaticGravity;
  winattrs.colormap = XCreateColormap(display,
    RootWindow(display, vinfo->screen),
    vinfo->visual, AllocNone);
  winattrs.override_redirect = False;

  window = XCreateWindow(display, RootWindow(display, vinfo->screen),
    0, 0, 1280, 720,
    0, vinfo->depth, InputOutput, vinfo->visual,
    CWBorderPixel | CWBitGravity | CWEventMask | CWColormap,
    &winattrs);
  CHECK(window, "XCreateWindow");

  XStoreName(display, window, "shapa");

  delete_message = XInternAtom(display, "WM_DELETE_WINDOW", True);
  XSetWMProtocols(display, window, &delete_message, 1);

  XMapWindow(display, window);

  context = glXCreateNewContext(display, glxconfigs[0], GLX_RGBA_TYPE, /*share_list*/ 0, True);
  CHECK(context, "glXCreateNewContext");

  drawable = glXCreateWindow(display, glxconfigs[0], window, 0);
  CHECK(drawable, "glXCreateWindow");

  glXMakeContextCurrent(display, drawable, drawable, context);

  XSelectInput(display, window, StructureNotifyMask | KeyReleaseMask);
}

int os_window_process() {
  while (XPending(display)) {
    XEvent e;
    XNextEvent(display, &e);
    switch (e.type) {
      case ConfigureNotify:
        shader_resize(e.xconfigure.width, e.xconfigure.height);
        break;

      case KeyRelease:
        if (XLookupKeysym(&e.xkey, 0) != XK_Escape)
          break;
      case ClientMessage:
      case DestroyNotify:
      case UnmapNotify:
        return 0;
    }
  }

  return 1;
}

void os_window_blit() {
  glXSwapBuffers(display, drawable);
}

void os_close_window() {
  glXMakeContextCurrent(display, 0, 0, 0);
  glXDestroyWindow(display, drawable);
  glXDestroyContext(display, context);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}

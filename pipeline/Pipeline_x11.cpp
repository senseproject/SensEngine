#include "Pipeline.hpp"
#include "GL/glew.h"
#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/Xlib.h>

#include <cstring>
#include <stdexcept>

struct PipelinePlatform {
  Window win;
  Display *dpy;
  GLXContext ctx;
  Colormap cmap;

  Display *loader_dpy;
  GLXPbuffer loader_pb;
  GLXContext loader_ctx;
};

namespace {
  Atom WM_PROTOCOLS;
  Atom WM_DELETE_WINDOW;
};

// Helper to check for extension string presence.  Adapted from:
//   http://www.opengl.org/resources/features/OGLextensions/
static bool isExtensionSupported(const char *extList, const char *extension) {
  const char *start;
  const char *where, *terminator;

  /* Extension names should not have spaces. */
  where = strchr(extension, ' ');
  if ( where || *extension == '\0' )
    return false;

  /* It takes a bit of care to be fool-proof about parsing the
   *    OpenGL extensions string. Don't be fooled by sub-strings,
   *    etc. */
  for ( start = extList; ; ) {
    where = strstr( start, extension );

    if ( !where )
      break;

    terminator = where + strlen( extension );

    if ( where == start || *(where - 1) == ' ' )
      if ( *terminator == ' ' || *terminator == '\0' )
        return true;

      start = terminator;
  }

  return false;
}

static Bool predicate(Display*, XEvent*, XPointer) {
  return 1;
}

static int visual_attribs[] = {
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
  GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, True,
  None
};

static int pbuffer_attribs[] = {
  GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
  None
};

static int context_attribs[] = {
  GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
  GLX_CONTEXT_MINOR_VERSION_ARB, 2,
  GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef NDEBUG
  GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#else
  GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
  None
};

PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribs = 0;

#undef glXChooseFBConfig
#undef glXGetVisualFromFBConfig

void Pipeline::platformInit() {
  platform = new PipelinePlatform;
  try {
  Display *dpy = platform->dpy = XOpenDisplay(0);
  if(!dpy)
    throw std::runtime_error("Could not open X11 display!");
  try { // display is open. If we get an error from here down we need to close the display

  // Verify our GLX version is the minimum to be doing our fun stuff
  int glx_major, glx_minor;
  if(!glXQueryVersion(dpy, &glx_major, &glx_minor) || ((glx_major == 1 && glx_minor < 3) || glx_major < 1))
    throw std::runtime_error("GLX 1.3 or greater is required!");

  // Query for an FBConfig that matches our requirements
  int fbcount;
  GLXFBConfig *cfgs = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount);
  if(!cfgs)
    throw std::runtime_error("No framebuffer that meets our requirements was found!");
  GLXFBConfig fbc = cfgs[0];
  XFree(cfgs);

  // Get the visual and set the window attributes
  XVisualInfo *vi = glXGetVisualFromFBConfig(dpy, fbc);
  if(!vi)
    throw std::runtime_error("Could not get a visual from the fbconfig");
  XSetWindowAttributes swa;
  swa.colormap = platform->cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
  swa.background_pixmap = None;
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask; // TODO: Add more event masks as needed

  // Create the window
  Window win = platform->win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel|CWColormap|CWEventMask, &swa);
  if(!win)
    throw std::runtime_error("Could not create X11 window");

  try { // everything from here on needs to close the window if there's an error

  XStoreName(dpy, win, "SensEngine Demo");
  XMapWindow(dpy, win);
  // Query if the needed GLX extensions are present
  const char *glx_ext_string = glXQueryExtensionsString(dpy, DefaultScreen(dpy));
  if(!isExtensionSupported(glx_ext_string, "GLX_ARB_create_context"))
    throw std::runtime_error("GLX_ARB_create_context needs to be supported");
  CreateContextAttribs = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
  if(!CreateContextAttribs)
    throw std::runtime_error("Could not get context creation entrypoint from GLX");

  GLXContext ctx = platform->ctx = CreateContextAttribs(dpy, fbc, 0, True, context_attribs);
  if(!ctx)
    throw std::runtime_error("Could not create the OpenGL context");
  try {
  if(!glXIsDirect(dpy, ctx))
    throw std::runtime_error("Can only use a direct context");
  glXMakeContextCurrent(dpy, win, win, ctx);
  } catch (std::exception&) {
    glXDestroyContext(dpy, ctx);
    throw;
  }
  } catch (std::exception&) {
    XDestroyWindow(dpy, win);
    throw;
  }
  } catch (std::exception&) {
    XCloseDisplay(dpy);
    throw;
  }
  } catch(std::exception&) {
    delete platform;
    platform = 0;
    throw;
  }

  // GLX initialization is done. Let's get our window to be a bit more modern
  WM_PROTOCOLS = XInternAtom(platform->dpy, "WM_PROTOCOLS", False);
  WM_DELETE_WINDOW = XInternAtom(platform->dpy, "WM_DELETE_WINDOW", False);
  Atom protocols[] = { WM_DELETE_WINDOW };
  XSetWMProtocols(platform->dpy, platform->win, protocols, 1);
}

void Pipeline::platformInitLoader() {
  Display *dpy = platform->loader_dpy = XOpenDisplay(0);
  if(!dpy)
    throw std::runtime_error("Loader thread could not open X Display");
  try {
  // Query for an FBConfig that matches our requirements
  int fbcount;
  GLXFBConfig *cfgs = glXChooseFBConfig(dpy, DefaultScreen(dpy), pbuffer_attribs, &fbcount);
  if(!cfgs)
    throw std::runtime_error("No framebuffer that meets our loader requirements was found!");
  GLXFBConfig fbc = cfgs[0];
  XFree(cfgs);

  GLXPbuffer pb = platform->loader_pb = glXCreatePbuffer(dpy, fbc, 0);
  if(!pb)
    throw std::runtime_error("Could not create the loader pbuffer");
  // from here on we need to destroy the pbuffer if we fail somewhere
  try {
  GLXContext ctx = platform->loader_ctx = CreateContextAttribs(dpy, fbc, platform->ctx, True, context_attribs);
  if(!ctx)
    throw std::runtime_error("Could not create the OpenGL context");
  try {
  if(!glXIsDirect(dpy, ctx))
    throw std::runtime_error("Loader context is indirect");
  glXMakeContextCurrent(dpy, pb, pb, ctx);
  } catch (std::exception&) {
    glXDestroyContext(dpy, ctx);
    throw;
  }
  } catch (std::exception&) {
    glXDestroyPbuffer(dpy, pb);
    throw;
  }
  } catch (std::exception&) {
    XCloseDisplay(dpy);
    throw;
  }
}

void Pipeline::platformFinish() {
  glXMakeContextCurrent(platform->dpy, 0, 0, 0);
  glXDestroyContext(platform->dpy, platform->ctx);
  XDestroyWindow(platform->dpy, platform->win);
  XFreeColormap(platform->dpy, platform->cmap);
  XCloseDisplay(platform->dpy);
  delete platform;
  platform = 0;
}

void Pipeline::platformFinishLoader() {
  glXMakeContextCurrent(platform->loader_dpy, 0, 0, 0);
  glXDestroyContext(platform->loader_dpy, platform->loader_ctx);
  glXDestroyPbuffer(platform->loader_dpy, platform->loader_pb);
  XCloseDisplay(platform->loader_dpy);
}

void Pipeline::platformSwap() {
  glXSwapBuffers(platform->dpy, platform->win);
}

void Pipeline::platformAttachContext() {
  glXMakeContextCurrent(platform->dpy, platform->win, platform->win, platform->ctx);
}

void Pipeline::platformDetachContext() {
  glXMakeContextCurrent(platform->dpy, 0, 0, 0);
}

bool Pipeline::platformEventLoop() {
  XEvent xevt;
  while(XCheckIfEvent(platform->dpy, &xevt, predicate, 0)) {
    switch(xevt.type) {
      case ClientMessage:
        if(xevt.xclient.message_type == WM_PROTOCOLS && (unsigned long)xevt.xclient.data.l[0] == WM_DELETE_WINDOW) {
          return false;
        }
        break;
      case ConfigureNotify: {
        int nwidth = xevt.xconfigure.width;
        int nheight = xevt.xconfigure.height;
        if(nwidth != width || nheight != height) {
          width = nwidth;
          height = nheight;
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          glViewport(0, 0, width, height);
          setupRenderTargets();
        }
        break;
      }
      default:
        break;
    }
  }
  return true;
}

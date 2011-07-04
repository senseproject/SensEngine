// Copyright 2011 Branan Purvine-Riley and Adam Johnson
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ClientPlatform_x11.hpp"
#include "Client.hpp"
#include "pipeline/interface.hpp"
#include "pipeline/ogl/glexcept.hpp"
#include <cstring>
#include <stdexcept>

namespace {
  Atom WM_PROTOCOLS;
  Atom WM_DELETE_WINDOW;
  
  // Helper to check for extension string presence.  Adapted from:
  //   http://www.opengl.org/resources/features/OGLextensions/
  bool isExtensionSupported(const char *extList, const char *extension) {
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

  Bool predicate(Display*, XEvent*, XPointer) {
    return 1;
  }

  int visual_attribs[] = {
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

  int pbuffer_attribs[] = {
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

  PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;

  uint32_t event_mask = StructureNotifyMask;
}

#undef glXChooseFBConfig
#undef glXGetVisualFromFBConfig
#undef glXCreateContextAttribsARB

void SenseClient::platformInit()
{
  m_platform_info = new ClientPlatform;
  try {
    Display* dpy = m_platform_info->dpy = XOpenDisplay(0);
    if (!dpy)
      throw std::runtime_error("Could not open X11 display");
    try { // Display is now open. We need to close it on any errors
      // verify our GLX version will work for creating a GL3 context
      int glx_major, glx_minor;
      if (!glXQueryVersion(dpy, &glx_major, &glx_minor) || ((glx_major == 1 && glx_minor < 3) || glx_major < 1))
        throw std::runtime_error("GLX 1.3 or greater is required");

      int fbcount;
      GLXFBConfig* cfgs = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount);
      if (!cfgs || !fbcount)
        throw std::runtime_error("No framebuffer configuration found.");
      GLXFBConfig fbc = cfgs[0];
      XFree(cfgs);
 
      XVisualInfo* vi = glXGetVisualFromFBConfig(dpy, fbc);
      if (!vi)
        throw std::runtime_error("Could not get visual information from framebuffer configuration");

      XSetWindowAttributes swa;
      swa.colormap = m_platform_info->cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
      swa.background_pixmap = None;
      swa.border_pixel = 0;
      swa.event_mask = event_mask;

      Window win = m_platform_info->win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, width(), height(), 0, vi->depth, InputOutput, vi->visual, CWBorderPixel|CWColormap|CWEventMask, &swa);
      if (!win)
        throw std::runtime_error("Could not create an OpenGL window");

      try { // Everything from here needs to close the window if there's an error
        // Setup the window bits
        XStoreName(dpy, win, displayName());
        XMapWindow(dpy, win);
 
        const char* glx_ext_string = glXQueryExtensionsString(dpy, DefaultScreen(dpy));
        if (!isExtensionSupported(glx_ext_string, "GLX_ARB_create_context"))
          throw std::runtime_error("No OpenGL 3 context support found for your GPU");
        glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
        if (!glXCreateContextAttribsARB)
          throw std::runtime_error("Could not get OpenGL 3 context creation entry point");

        GLXContext ctx = m_platform_info->ctx = glXCreateContextAttribsARB(dpy, fbc, 0, True, context_attribs);
        if (!ctx)
          throw std::runtime_error("Could not create an OpenGL 3 context");
        if (!glXIsDirect(dpy, ctx)) {
          glXDestroyContext(dpy, ctx);
          throw std::runtime_error("Indirect rendering is not supported.");
        }
        glXMakeContextCurrent(dpy, win, win, ctx);
      } catch (std::exception&) {
        XDestroyWindow(dpy, win);
        throw;
      }
    } catch (std::exception&) {
      XCloseDisplay(dpy);
      throw;
    }
  } catch (std::exception&) {
    delete m_platform_info;
    m_platform_info = 0;
    throw;
  }

  WM_PROTOCOLS = XInternAtom(m_platform_info->dpy, "WM_PROTOCOLS", False);
  WM_DELETE_WINDOW = XInternAtom(m_platform_info->dpy, "WM_DELETE_WINDOW", False);
  Atom protocols[] = { WM_DELETE_WINDOW };
  XSetWMProtocols(m_platform_info->dpy, m_platform_info->win, protocols, 1);

  glewExperimental = GL_TRUE;
  glewInit();

  GL_CHECK(glViewport(0, 0, m_width, m_height));
}

void SenseClient::platformInitLoader() 
{
  Display *dpy = m_platform_info->loader_dpy = XOpenDisplay(0);
  if (!dpy)
    throw std::runtime_error("Loader thread could not open X Display");
  try {
    int fbcount;
    GLXFBConfig *cfgs = glXChooseFBConfig(dpy, DefaultScreen(dpy), pbuffer_attribs, &fbcount);
    if (!fbcount || !cfgs)
      throw std::runtime_error("No framebuffer that meets loader requirements found");
    GLXFBConfig fbc = cfgs[0];
    XFree(cfgs);
    
    GLXPbuffer pb = m_platform_info->loader_pb = glXCreatePbuffer(dpy, fbc, 0);
    if (!pb)
      throw std::runtime_error("Could not create pbuffer for loader thread");
    try {
      GLXContext ctx = m_platform_info->loader_ctx = glXCreateContextAttribsARB(dpy, fbc, m_platform_info->ctx, True, context_attribs);
      if (!ctx)
        throw std::runtime_error("Could not create OpenGL context for loader thread");
      if (!glXIsDirect(dpy, ctx)) {
        glXDestroyContext(dpy, ctx);
        throw std::runtime_error("Loader context cannot be indirect");
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

void SenseClient::platformFinish() 
{
  glXMakeContextCurrent(m_platform_info->dpy, 0, 0, 0);
  glXDestroyContext(m_platform_info->dpy, m_platform_info->ctx);
  XDestroyWindow(m_platform_info->dpy, m_platform_info->win);
  XFreeColormap(m_platform_info->dpy, m_platform_info->cmap);
  XCloseDisplay(m_platform_info->dpy);
  delete m_platform_info;
  m_platform_info = 0;
}

void SenseClient::platformFinishLoader()
{
  glXMakeContextCurrent(m_platform_info->loader_dpy, 0, 0, 0);
  glXDestroyContext(m_platform_info->loader_dpy, m_platform_info->loader_ctx);
  glXDestroyPbuffer(m_platform_info->loader_dpy, m_platform_info->loader_pb);
  XCloseDisplay(m_platform_info->loader_dpy);
}

void SenseClient::platformSwapBuffers() 
{
  
  glXSwapBuffers(m_platform_info->dpy, m_platform_info->win);
}

void SenseClient::platformAttachContext() 
{
  
  glXMakeContextCurrent(m_platform_info->dpy, m_platform_info->win, m_platform_info->win, m_platform_info->ctx);
}

void SenseClient::platformDetachContext() {
  glXMakeContextCurrent(m_platform_info->dpy, 0, 0, 0);
}

bool SenseClient::platformEventLoop()
{
  XEvent xevt;
  while(XCheckIfEvent(m_platform_info->dpy, &xevt, predicate, 0)) {
    switch(xevt.type) {
      case ClientMessage:
        if(xevt.xclient.message_type == WM_PROTOCOLS && (unsigned long)xevt.xclient.data.l[0] == WM_DELETE_WINDOW) {
          return false;
        }
        break;
      case ConfigureNotify: {
        m_new_width = xevt.xconfigure.width;
        m_new_height = xevt.xconfigure.height;
        break;
      }
      default:
        break;
    }
  }
  return true;
}


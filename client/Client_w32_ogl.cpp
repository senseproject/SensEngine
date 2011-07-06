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

#include "Client.hpp"
#include "ClientPlatform_w32.hpp"
#include "pipeline/interface.hpp"
#include "pipeline/ogl/glexcept.hpp"

#undef wglChoosePixelFormatARB
#undef wglCreateContextAttribsARB

// This should be in an anonymous namespace, but that apparently breaks friends on MSVC. Oh well.
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  SenseClient *client = (SenseClient*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  switch(msg) {
    case WM_CLOSE:
      client->platformWndProcReturn = false;
      return 0;
    case WM_SIZE:
      client->m_new_width = LOWORD(lParam);
      client->m_new_height = HIWORD(lParam);
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

namespace {
  bool wndclass_defined = false;
  uint32_t wndclass_users = 0;

  int format_attribs[] = {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
    WGL_COLOR_BITS_ARB, 24,
    WGL_DEPTH_BITS_ARB, 24,
    WGL_STENCIL_BITS_ARB, 8,
    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
    WGL_FRAMEBUFFER_SRGB_CAPABLE_EXT, GL_TRUE,
    0, 0
  };

  int context_attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 2,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef NDEBUG
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#else
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
    0
  };

  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
}

void SenseClient::platformInit()
{
  WNDCLASS wc;
  HWND hwnd;
  HDC hdc;
  HGLRC ctx;
  HGLRC ctx2;
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = (HINSTANCE)GetModuleHandle(0);
  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = 0;
  wc.lpszMenuName = 0;
  wc.lpszClassName = windowClass();

  if(!wndclass_defined) {
    if (!RegisterClass(&wc))
      throw std::runtime_error("Could not register window class");
  } else {
    wndclass_users++;
  }
  try {
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    RECT window_rect;
    window_rect.left = window_rect.top = 0;
    window_rect.right = width();
    window_rect.bottom = height();
    AdjustWindowRectEx(&window_rect, dwStyle, FALSE, dwExStyle);

    LONG h = window_rect.bottom - window_rect.top;
    LONG w = window_rect.right - window_rect.left;
    hwnd = CreateWindowEx(dwExStyle, windowClass(), displayName(), dwStyle, 0, 0, w, h, NULL, NULL, GetModuleHandle(0), NULL);
    if (!hwnd)
      throw std::runtime_error("Could not create application window");
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

    try {
      hdc = GetDC(hwnd);
      if (!hdc)
        throw std::runtime_error("Could not get a valid device handle from the window");

      try {
        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int pf = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, pf, &pfd);

        ctx = wglCreateContext(hdc);
        if (!ctx)
          throw std::runtime_error("Could not create a temporary rendering context");

        wglMakeCurrent(hdc, ctx);
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

        wglMakeCurrent(0, 0);
        wglDeleteContext(ctx);

        unsigned int num_results = 0;
        pf = 0;

        if (!wglCreateContextAttribsARB || ! wglChoosePixelFormatARB)
          throw std::runtime_error("Could not get OpenGL 3 context entry points");

        BOOL valid = wglChoosePixelFormatARB(hdc, format_attribs, NULL, 1, &pf, &num_results);
        if (!valid || !num_results)
          throw std::runtime_error("Could not get a valid pixel format");

        DescribePixelFormat(hdc, pf, sizeof(pfd), &pfd);
        SetPixelFormat(hdc, pf, &pfd);

        ctx = wglCreateContextAttribsARB(hdc, NULL, context_attribs);
        ctx2 = wglCreateContextAttribsARB(hdc, ctx, context_attribs);

        if (!ctx) {
          if(ctx2)
            wglDeleteContext(ctx2);
          throw std::runtime_error("Could not create OpenGL 3 context");
        }
        if (!ctx2) {
          if(ctx)
            wglDeleteContext(ctx);
          throw std::runtime_error("Could not create OpenGL 3 loader context");
        }

        if(!wglMakeCurrent(hdc, ctx)) {
          wglDeleteContext(ctx);
          wglDeleteContext(ctx2);
          throw std::runtime_error("Could not make context current");
        }
      } catch (std::exception&) {
        ReleaseDC(hwnd, hdc);
        throw;
      }
    } catch (std::exception&) {
      DestroyWindow(hwnd);
      throw;
    }
  } catch (std::exception&) {
    wndclass_users--;
    if(!wndclass_users) {
      UnregisterClass(windowClass(), GetModuleHandle(0));
    }
    throw;
  }

  m_platform_info = new ClientPlatform;
  m_platform_info->hdc = hdc;
  m_platform_info->hwnd = hwnd;
  m_platform_info->ctx = ctx;
  m_platform_info->loader_ctx = ctx2;

  ShowWindow(hwnd, SW_SHOW);
  SetForegroundWindow(hwnd);
  SetFocus(hwnd);

  glewExperimental = GL_TRUE;
  glewInit();

  GL_CHECK(glViewport(0, 0, m_width, m_height));
}

void SenseClient::platformInitLoader()
{
  if(!wglMakeCurrent(m_platform_info->hdc, m_platform_info->loader_ctx))
    throw std::runtime_error("Could not make loader context current");
}

void SenseClient::platformFinish()
{
  wglMakeCurrent(0, 0);
  wglDeleteContext(m_platform_info->ctx);
  wglDeleteContext(m_platform_info->loader_ctx);
  ReleaseDC(m_platform_info->hwnd, m_platform_info->hdc);
  DestroyWindow(m_platform_info->hwnd);
  wndclass_users--;
  if(!wndclass_users)
    UnregisterClass(windowClass(), GetModuleHandle(0));
}

void SenseClient::platformFinishLoader()
{
  wglMakeCurrent(0, 0);
}


void SenseClient::platformSwapBuffers()
{
  SwapBuffers(m_platform_info->hdc);
}

void SenseClient::platformAttachContext()
{
  wglMakeCurrent(m_platform_info->hdc, m_platform_info->ctx);
}

void SenseClient::platformDetachContext()
{
  wglMakeCurrent(0, 0);
}

bool SenseClient::platformEventLoop()
{
  MSG msg;
  platformWndProcReturn = true;
  while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE) && platformWndProcReturn) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return platformWndProcReturn;
}

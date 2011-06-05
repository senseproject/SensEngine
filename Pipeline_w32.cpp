#include <Windows.h>
#include "GL/glew.h"
#include "GL/wglew.h"

#include "Pipeline.hpp"

struct PipelinePlatform {
  HWND win;
  HDC  dc;
  HGLRC ctx;
  HGLRC loader_ctx;
};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch(msg) {
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

int format_attribs[] = {
  WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
  WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
  WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
  WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
  WGL_DEPTH_BITS_ARB, 24,
  WGL_STENCIL_BITS_ARB, 8,
  WGL_RED_BITS_ARB, 8,
  WGL_GREEN_BITS_ARB, 8,
  WGL_BLUE_BITS_ARB, 8,
  WGL_ALPHA_BITS_ARB, 8,
  WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
  WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE
};

static int context_attribs[] = {
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

PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribs = 0;
PFNWGLCHOOSEPIXELFORMATARBPROC ChoosePixelFormatARB = 0;

void Pipeline::platformInit() {
  HWND hwnd;
  HDC hdc;
  HGLRC ctx1;
  HGLRC ctx2;
  WNDCLASS wc;
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = (HINSTANCE)GetModuleHandle(0);
  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = 0;
  wc.lpszMenuName = 0;
  wc.lpszClassName = "SenseEngineMainWindow";

  if(!RegisterClass(&wc))
    throw std::runtime_error("Could not register window class");

  try { // from here on we need to clean up the window class if there's an error
  DWORD dwStyle = WS_OVERLAPPEDWINDOW;
  DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
  RECT window_rect;
  window_rect.left = 0;
  window_rect.top = 0;
  window_rect.right = width;
  window_rect.bottom = height;
  AdjustWindowRectEx(&window_rect, dwStyle, FALSE, dwExStyle);

  hwnd = CreateWindowEx(dwExStyle, "SenseEngineMainWindow", "Deferred Renderer Demo", dwStyle, 0, 0,
                             window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
                             NULL, NULL, GetModuleHandle(0), NULL);
  if(!hwnd)
    throw std::runtime_error("Could not create window!");

  try { // from now on we need to cleanup the main window if there's an error
  PIXELFORMATDESCRIPTOR tmp_pfd;
  memset(&tmp_pfd, 0, sizeof(tmp_pfd));
  tmp_pfd.nSize = sizeof(tmp_pfd);
  tmp_pfd.nVersion = 1;
  tmp_pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
  tmp_pfd.iPixelType = PFD_TYPE_RGBA;
  tmp_pfd.cColorBits = 32;
  tmp_pfd.cDepthBits = 32;
  tmp_pfd.iLayerType = PFD_MAIN_PLANE;

  hdc = GetDC(hwnd);
  if(!hdc)
    throw std::runtime_error("Could not get device context from main window");

  int tmp_pf = ChoosePixelFormat(hdc, &tmp_pfd);
  if(!tmp_pf)
    throw std::runtime_error("Could not get a valid temporary pixel format");

  SetPixelFormat(hdc, tmp_pf, &tmp_pfd);

  HGLRC tmp_ctx = wglCreateContext(hdc);
  if(!tmp_ctx)
    throw std::runtime_error("Could not get a valid temporary OpenGL context");

  wglMakeCurrent(hdc, tmp_ctx);
  CreateContextAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
  ChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
  wglMakeCurrent(0, 0);
  wglDeleteContext(tmp_ctx);
  unsigned int num_results;
  int new_pf;

  if(!ChoosePixelFormatARB || !CreateContextAttribs)
    throw std::runtime_error("Could not get gl3 context entrypoints");

  ChoosePixelFormatARB(hdc, format_attribs, NULL, 1, &new_pf, &num_results);

  if(!new_pf)
    throw std::runtime_error("Could not get a pixel format for the window");

  DescribePixelFormat(hdc, new_pf, sizeof(tmp_pfd), &tmp_pfd);
  SetPixelFormat(hdc, new_pf, &tmp_pfd);

  ctx1 = CreateContextAttribs(hdc, 0, context_attribs);
  ctx2 = CreateContextAttribs(hdc, ctx1, context_attribs);

  if(!ctx1 || !ctx2)
    throw std::runtime_error("Could not create render and loader contexts");

  if(!wglMakeCurrent(hdc, ctx1))
    throw std::runtime_error("Could not make final context current");

  } catch (std::exception&) {
    DestroyWindow(hwnd);
    throw;
  }
  } catch (std::exception&) {
    UnregisterClass("SenseEngineMainWindow", (HINSTANCE)GetModuleHandle(0));
    throw;
  }

  ShowWindow(hwnd, SW_SHOW);
  SetForegroundWindow(hwnd);
  SetFocus(hwnd);

  platform = new PipelinePlatform;
  platform->dc = hdc;
  platform->ctx = ctx1;
  platform->loader_ctx = ctx2;
  platform->win = hwnd;
}

void Pipeline::platformInitLoader() {
  if(!wglMakeCurrent(platform->dc, platform->loader_ctx))
    throw std::runtime_error("Could not make context current in loader thread");
}

void Pipeline::platformFinish() {
  wglMakeCurrent(0, 0);
  wglDeleteContext(platform->ctx);
  wglDeleteContext(platform->loader_ctx);
  ReleaseDC(platform->win, platform->dc);
  DestroyWindow(platform->win);
}

void Pipeline::platformFinishLoader() {
  wglMakeCurrent(0, 0);
}

void Pipeline::platformAttachContext() {
  wglMakeCurrent(platform->dc, platform->ctx);
}

void Pipeline::platformDetachContext() {
  wglMakeCurrent(0, 0);
}

void Pipeline::platformSwap() {
  SwapBuffers(platform->dc);
}

bool Pipeline::platformEventLoop() {
  MSG msg;
  while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
    if(msg.message == WM_QUIT)
      return false;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return true;
}
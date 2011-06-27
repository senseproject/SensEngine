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

#include "Webview.hpp"
#include <berkelium/Berkelium.hpp>
#include <berkelium/Context.hpp>
#include <berkelium/Window.hpp>
#include <berkelium/ScriptUtil.hpp>

#include <iostream>
#include <cstring>

#include "GL/glew.h"
#include "glexcept.hpp"

Webview::Webview(std::string page, int w, int h) {
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, gl_texid))
  unsigned int black = 0x00000000;
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &black))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
  hres = w;
  vres = h;
  needs_full_refresh=true;
  Berkelium::Context *context = Berkelium::Context::create();
  window = Berkelium::Window::create(context);
  delete context;
  window->setDelegate(this);
  window->resize(w, h);
  window->setTransparent(true);
  window->focus();
  window->navigateTo(page.c_str(), page.size());
  scroll_buffer = new char[w*(h+1)*4];
}

Webview::~Webview() {
  delete window;
  delete[] scroll_buffer;
}

void Webview::onPaint(Berkelium::Window *wini, const unsigned char *bitmap_in, const Berkelium::Rect &bitmap_rect, size_t num_copy_rects, const Berkelium::Rect *copy_rects, int dx, int dy, const Berkelium::Rect &scroll_rect) {
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, gl_texid))
  if(needs_full_refresh) {
    if(bitmap_rect.left() != 0 || bitmap_rect.top() != 0 || bitmap_rect.right() != hres || bitmap_rect.bottom() != vres)
      return;
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hres, vres, 0, GL_BGRA, GL_UNSIGNED_BYTE, bitmap_in))
    needs_full_refresh = false;
    return;
  }

  if(dx != 0 || dy != 0) {
    Berkelium::Rect scrolled_rect = scroll_rect.translate(-dx, -dy);
    Berkelium::Rect scrolled_shared_rect = scroll_rect.intersect(scrolled_rect);
    std::cout << "Scrolling is unimplemented! come back later" << std::endl;
  }
  for(size_t i = 0; i < num_copy_rects; i++) {
    int wid = copy_rects[i].width();
    int hig = copy_rects[i].height();
    int top = copy_rects[i].top() - bitmap_rect.top();
    int left = copy_rects[i].left() - bitmap_rect.left();
    for(int jj = 0; jj < hig; jj++) {
      memcpy(scroll_buffer + jj*wid*4, bitmap_in+(left+(jj+top)*bitmap_rect.width())*4, wid*4);
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, copy_rects[i].left(), copy_rects[i].top(), wid, hig, GL_BGRA, GL_UNSIGNED_BYTE, scroll_buffer);
  }
  return;
}

void Webview::onCreatedWindow(Berkelium::Window* win, Berkelium::Window* newWindow, const Berkelium::Rect& initialRect) {
  if (initialRect.mWidth < 1 || initialRect.mHeight < 1)
    newWindow->resize(hres, vres);
  newWindow->setDelegate(this);
}

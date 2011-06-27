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

#ifndef SENSE_PIPELINE_WEBVIEW_HPP
#define SENSE_PIPELINE_WEBVIEW_HPP

#include "Texture.hpp"
#include <berkelium/WindowDelegate.hpp>

namespace Berkelium {
  struct Rect;
  class Window;
};

class Webview : public Texture, public Berkelium::WindowDelegate  {
public:
  virtual ~Webview();
  Webview(std::string page, int w, int h);

  virtual void onPaint(Berkelium::Window*, const unsigned char*, const Berkelium::Rect&, size_t, const Berkelium::Rect*, int, int, const Berkelium::Rect&);
  virtual void onCreatedWindow(Berkelium::Window*, Berkelium::Window*, const Berkelium::Rect&);

private:
  bool needs_full_refresh;
  Berkelium::Window* window;
  char* scroll_buffer;
};

#endif // SENSE_PIPELINE_WEBVIEW_HPP

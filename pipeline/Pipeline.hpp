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

#ifndef SENSE_PIPELINE_PIPELINE_HPP
#define SENSE_PIPELINE_PIPELINE_HPP

#include <cstdint>
#include <memory>
#include <set>
#include <vector>
#include "util/util.hpp"
#include "Loader.hpp"


GCC_DIAGNOSTIC_PUSH
GCC_DISABLE_WARNING("-Wunused-variable")
#include <boost/thread.hpp>
GCC_DIAGNOSTIC_POP

#include "util/queue.hpp"

#include <set>
#include <deque>

#include "3rdparty/glm/glm.hpp"

#define HANDLE(type) typedef std::shared_ptr<type> h##type;

struct PipelinePlatform;

class DrawBuffer;
class Material;
class RenderTarget;
class Texture;

struct Panel;

class Pipeline {
public:
  HANDLE(Material)
  HANDLE(DrawBuffer)
  HANDLE(RenderTarget)
  HANDLE(Panel)

  struct KbdCallback {
    enum Keysym {
    };
    virtual void operator()(Keysym, uint32_t) =0;
  };
      

private:
  HANDLE(Texture)

  struct GpuMemAvailable {
    int texture;
    int render;
    int buffer;
  };
  PipelinePlatform *platform;

  // loader stuff
  Loader l;
  boost::thread loader_thread;
  volatile bool loader_init_complete;
  volatile bool loader_be_done;
  std::string loader_error_string;

  // global OpenGL resources
  hTexture csm_tex_array;
  hTexture shadowmap;
  hRenderTarget default_framebuffer;
  hRenderTarget current_framebuffer;
  hRenderTarget gbuffer_framebuffer;

  // Various OpenGL limits and settings
  std::set<int> fsaa_levels;
  int cur_fsaa;
  int shadowmap_resolution;
  int num_csm_splits;

  // non-opengl settings
  int width, height;

  // misc pipeline data
  std::deque<glm::mat4> projection_stack;
  std::set<hPanel> panels;

  GpuMemAvailable queryAvailableMem();

  void platformInit();
  void platformInitLoader();
  void platformFinish();
  void platformFinishLoader();
  void platformSwap();
  void platformDetachContext();
  void platformAttachContext();

  void setupRenderTargets();
  int useMaterial(hMaterial);

#ifdef WIN32
  bool platformWndProcReturn;
  void platformSetWndProcRet(bool b) { platformWndProcReturn = b; }
#endif

protected:
  void runLoaderThread();

  hTexture createTargetTexture();
  hRenderTarget createRenderTarget();

public:
  Pipeline();
  ~Pipeline();

  void pushKbdCallback(KbdCallback*);

  void beginFrame();
  void render();
  void endFrame();

  bool platformEventLoop();

  void setRenderTarget(hRenderTarget);
  hRenderTarget buildRenderTarget(int w, int h, bool mip=true);

  Loader& loader() { return l; }

  // functions to set rendering options (FSAA, ANISO, &c)
  const std::set<int>& fsaaLevels() const { return fsaa_levels; }
  void setFsaa(int i) { if(fsaa_levels.find(i) != fsaa_levels.end()) { cur_fsaa = i; setupRenderTargets(); }}

  // Create and destroy panels
  hPanel createPanel(float, float, std::string);
  void destroyPanel(hPanel);

  // Play with the projection matrix
  void pushProjMat(glm::mat4 m) {projection_stack.push_front(m);}
  void popProjMat() {projection_stack.pop_front();}
  
      

#ifdef WIN32
  friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#endif
};

#endif // SENSE_PIPELINE_PIPELINE_HPP

#ifndef SENSE_PIPELINE_PIPELINE_HPP
#define SENSE_PIPELINE_PIPELINE_HPP

#include <cstdint>
#include <memory>
#include <set>
#include <vector>
#include "util/util.hpp"


GCC_DIAGNOSTIC_PUSH
GCC_DISABLE_WARNING("-Wunused-variable")
#include <boost/thread.hpp>
GCC_DIAGNOSTIC_POP

#include "util/queue.hpp"

#define HANDLE(type) typedef std::shared_ptr<type> h##type;

struct PipelinePlatform;

class Drawbuffer;
class Material;
class RenderTarget;
class Texture;

class Pipeline {
public:
  HANDLE(Material)
  HANDLE(Drawbuffer)
  HANDLE(RenderTarget)

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
  typedef std::pair<uint32_t, hTexture> TexLoaderMsg;
  typedef std::pair<uint32_t, hDrawbuffer> BufLoaderMsg;
  queue<TexLoaderMsg> texloader_queue;
  queue<BufLoaderMsg> bufloader_queue;
  boost::thread loader_thread;
  volatile bool loader_init_complete;
  volatile bool loader_be_done;
  std::string loader_error_string;

  // global OpenGL resources
  hTexture default_texture;
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

  GpuMemAvailable queryAvailableMem();

  void platformInit();
  void platformInitLoader();
  void platformFinish();
  void platformFinishLoader();
  void platformSwap();
  void platformDetachContext();
  void platformAttachContext();

  void setupRenderTargets();

#ifdef WIN32
  bool platformWndProcReturn;
  void platformSetWndProcRet(bool b) { platformWndProcReturn = b; }
#endif

protected:
  void runLoaderThread();

  void deleteTexture(Texture*);
  void deleteTargetTexture(Texture*);
  void deleteRenderTarget(RenderTarget*);

  hTexture createTexture(std::string);
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

  // functions to set rendering options (FSAA, ANISO, &c)
  const std::set<int>& fsaaLevels() const { return fsaa_levels; }
  void setFsaa(int i) { if(fsaa_levels.find(i) != fsaa_levels.end()) { cur_fsaa = i; setupRenderTargets(); }}

  // These are all used as callbacks
  friend struct TextureDealloc;
  friend struct TargetTextureDealloc;
  friend struct RenderTargetDealloc;

#ifdef WIN32
  friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#endif
};

#endif // SENSE_PIPELINE_PIPELINE_HPP

#ifndef Pipeline_hpp
#define Pipeline_hpp

#include <cstdint>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

#include <boost/thread.hpp>

#include "queue.hpp"

#define HANDLE(type) typedef std::shared_ptr<type> h##type;

struct PipelinePlatform;

class Pipeline {
public:
  struct KbdCallback {
    enum Keysym {
    };
    virtual void operator()(Keysym, uint32_t) =0;
  };

  struct Texture {
    volatile uint32_t id;
    volatile int biggest_mip_loaded;
    std::string name;
    // Other special properties about this texture
  };
  HANDLE(Texture)

  struct RenderTarget {
    volatile uint32_t id;
    std::vector<hTexture> bound_textures;
  };
  HANDLE(RenderTarget)

private:

  struct GpuMemAvailable {
    int texture;
    int render;
    int buffer;
  };
  PipelinePlatform *platform;

  // loader stuff
  typedef std::pair<uint32_t, void*> LoaderMsg;
  queue<LoaderMsg> loader_queue;
  boost::thread loader_thread;
  bool loader_init_complete;
  std::string loader_error_string;

  // global OpenGL resources
  hTexture default_texture;
  hRenderTarget default_framebuffer;
  hRenderTarget current_framebuffer;
  hRenderTarget gbuffer_framebuffer;

  // Various OpenGL limits and settings
  std::set<int> fsaa_levels;
  int cur_fsaa;

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

  // functions to set rendering options (FSAA, ANISO, &c)
  const std::set<int>& fsaaLevels() const { return fsaa_levels; }
  void setFsaa(int i) { if(fsaa_levels.find(i) != fsaa_levels.end()) { cur_fsaa = i; setupRenderTargets(); }}

  // These are all used as callbacks
  friend void launchLoaderThread(Pipeline*);
  friend struct TextureDealloc;
  friend struct TargetTextureDealloc;
  friend struct RenderTargetDealloc;
};

#endif // Pipeline_hpp

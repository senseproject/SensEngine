#ifndef Pipeline_hpp
#define Pipeline_hpp

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <vector>
#include <memory>

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
  std::thread loader_thread;
  bool loader_init_complete;
  std::string loader_error_string;

  hRenderTarget default_framebuffer;
  hRenderTarget current_framebuffer;
  hRenderTarget gbuffer_framebuffer;

  // Various OpenGL limits and settings
  int max_fsaa;
  int fsaa_count;

  GpuMemAvailable queryAvailableMem();

  void platformInit();
  void platformInitLoader();
  void platformFinish();
  void platformSwap();
  void platformDetachContext();
  void platformAttachContext();

protected:
  void runLoaderThread();

  void deleteTexture(Texture*);
  void deleteTargetTexture(Texture*);
  void deleteRenderTarget(RenderTarget*);

  hTexture createTargetTexture();
  hRenderTarget createRenderTarget();

public:
  Pipeline();
  ~Pipeline();

  void pushKbdCallback(KbdCallback*);

  void beginFrame();
  void render();
  void endFrame();

  void setRenderTarget(hRenderTarget);

  // These are all used as callbacks
  friend void launchLoaderThread(Pipeline*);
  friend struct TextureDealloc;
  friend struct TargetTextureDealloc;
  friend struct RenderTargetDealloc;
};

#endif // Pipeline_hpp

#include <GL/glew.h>

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <cstdio>

#include "Pipeline.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "util.hpp"

// Loader message types
enum LoaderMessages {
  LoaderMsgLoadTexture,
  LoaderMsgUnloadTexture,
};

// Builtin texture data
unsigned int tex_builtin_default[] =
{
  0x00555555, 0x00555555, 0x00555555, 0x00555555, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA,
  0x00555555, 0x00555555, 0x00555555, 0x00555555, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA,
  0x00555555, 0x00555555, 0x00555555, 0x00555555, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA,
  0x00555555, 0x00555555, 0x00555555, 0x00555555, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA,
  0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00555555, 0x00555555, 0x00555555, 0x00555555,
  0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00555555, 0x00555555, 0x00555555, 0x00555555,
  0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00555555, 0x00555555, 0x00555555, 0x00555555,
  0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA, 0x00555555, 0x00555555, 0x00555555, 0x00555555,
};

static const char* glErrorString(GLenum err) {
  switch(err) {
    case GL_INVALID_ENUM: return "Invalid Enum";
    case GL_INVALID_VALUE: return "Invalid Value";
    case GL_INVALID_OPERATION: return "Invalid Operation";
    case GL_STACK_OVERFLOW: return "Stack Overflow";
    case GL_STACK_UNDERFLOW: return "Stack Underflow";
    case GL_OUT_OF_MEMORY: return "Out of Memory";
    case GL_TABLE_TOO_LARGE: return "Table too Large";
    default: return "Unknown Error";
  }
}

static const char* fboErrorString(GLenum status) {
  switch(status) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "Incomplete Attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "Missing Attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "Incomplete Draw Buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "Incomplete Read Buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED: return "Unsupposed Configuration";
    default: return "Unknown Error";
  }
}

class gl_error : public std::runtime_error {
public:
  gl_error(std::string location, GLenum err) : std::runtime_error(glErrorString(err)+location) {}
};

class fbo_error : public std::runtime_error {
public:
  fbo_error(std::string location, GLenum status) : std::runtime_error(fboErrorString(status)+location) {}
};

#ifndef NDEBUG
#define GL_CHECK(func) func; { GLenum glerr = glGetError(); if(GL_NO_ERROR != glerr) throw gl_error(FILE_LINE, glerr); }
#define FBO_CHECK { GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); if(GL_FRAMEBUFFER_COMPLETE != status) throw fbo_error(FILE_LINE, status); }
#else
#define GL_CHECK(func) func;
#define FBO_CHECK
#endif

struct TextureDealloc {
  Pipeline *pipe;
  TextureDealloc(Pipeline *pipe) : pipe(pipe) {}
  void operator()(Texture *p) { pipe->deleteTexture(p); }  
};

struct TargetTextureDealloc {
  Pipeline *pipe;
  TargetTextureDealloc(Pipeline *pipe) : pipe(pipe) {}
  void operator()(Texture *p) { pipe->deleteTargetTexture(p); }
};

struct RenderTargetDealloc {
  Pipeline *pipe;
  RenderTargetDealloc(Pipeline *pipe) : pipe(pipe) {}
  void operator()(RenderTarget *p) { pipe->deleteRenderTarget(p); }
};

void Pipeline::runLoaderThread() {
  try {
  platformInitLoader();
  } catch (std::exception& e) {
    loader_error_string = e.what();
  }
  loader_init_complete = true;

  while(!loader_be_done) {
    bool loaded_objects_this_loop = false;
    for(BufLoaderMsg bmsg; bufloader_queue.try_pop(bmsg);) {
      if(loader_be_done) break;
      //TODO: check for various buffer load messages
      loaded_objects_this_loop = true;
    };
    if(loaded_objects_this_loop) {
      // TODO: add a thread condition to signal object loads are complete
    }
    TexLoaderMsg tmsg;
    if(!loader_be_done && texloader_queue.try_pop(tmsg)) {
      switch(tmsg.first) {
        case LoaderMsgLoadTexture: {
          throw std::logic_error("Texture loading is not yet implemented!");
        }
        case LoaderMsgUnloadTexture: {
          // Automagically deleted when tmsg goes out of scope
          break;
        }
      }
    } else if(!loader_be_done) {
      boost::thread::yield(); // there's nothing on the queue. Let's be a good citizen
    }
  }

  for(BufLoaderMsg bmsg; bufloader_queue.try_pop(bmsg);) {
    // TODO: run just buffer unload messages
  }

  for(TexLoaderMsg tmsg; texloader_queue.try_pop(tmsg);) ; // delete messages will work like magic here because of the shared_ptr

  platformFinishLoader();
}

void Pipeline::deleteTexture(Texture *p) {
  texloader_queue.push(TexLoaderMsg(LoaderMsgUnloadTexture, hTexture(p)));
}

void Pipeline::deleteTargetTexture(Texture* p) {
  delete p;
}

void Pipeline::deleteRenderTarget(RenderTarget* p) {
  delete p;
}

Pipeline::hTexture Pipeline::createTexture(std::string name) {
  hTexture tex = hTexture(new Texture, TextureDealloc(this));
  tex->name = name;
  tex->biggest_mip_loaded=-1;
  texloader_queue.push(TexLoaderMsg(LoaderMsgLoadTexture, tex));
  return tex;
}

Pipeline::hTexture Pipeline::createTargetTexture() {
  hTexture tex(new Texture, TargetTextureDealloc(this));
  return tex;
}

Pipeline::hRenderTarget Pipeline::createRenderTarget() {
  hRenderTarget target(new RenderTarget, RenderTargetDealloc(this));
  target->build_mips = true; // default to building mipmaps
  return target;
}

Pipeline::Pipeline() : loader_init_complete(false), loader_be_done(false), shadowmap_resolution(512), num_csm_splits(3), width(800), height(600) {
  platformInit();
  glewExperimental = GL_TRUE;
  glewInit();

  platformDetachContext(); // On X11, the context can't be current when we setup the loader thread
  loader_thread = boost::thread(std::mem_fun(&Pipeline::runLoaderThread), this);
  while(!loader_init_complete);
  platformAttachContext();
  if(!loader_error_string.empty()) {
    loader_thread.join();
    platformFinish();
    throw std::runtime_error(loader_error_string.c_str());
  }

  int dsamples, csamples;
  glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &dsamples);
  glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &csamples);
  int fsaa = std::min(dsamples, csamples);
  cur_fsaa = fsaa;
  do {
    fsaa_levels.insert(fsaa);
    fsaa /= 2;
  } while(fsaa > 1);
  

  // Enable some standard state
  GL_CHECK(glEnable(GL_MULTISAMPLE))
  GL_CHECK(glEnable(GL_FRAMEBUFFER_SRGB))
  glClearColor(0.8f, 0.8f, 0.9f, 1.f);

  // Create our FBOs
  default_framebuffer = createRenderTarget();
  default_framebuffer->build_mips = false;
  gbuffer_framebuffer = createRenderTarget();
  gbuffer_framebuffer->build_mips = false;
  setupRenderTargets();
  setRenderTarget(default_framebuffer);

  // Create a default texture as a placeholder when we're streaming something in
  hTexture default_texture = createTargetTexture(); // not really a render target, but this gives us a main-thread-owned texture
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, default_texture->gl_texid))
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_builtin_default))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))

  csm_tex_array = createTargetTexture();
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, csm_tex_array->gl_texid))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE))
  GL_CHECK(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, shadowmap_resolution, shadowmap_resolution, num_csm_splits+1, 0, GL_RED, GL_FLOAT, 0))

  shadowmap = createTargetTexture();
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, shadowmap->gl_texid))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE))
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, shadowmap_resolution, shadowmap_resolution, 0, GL_RED, GL_FLOAT, 0))
}

Pipeline::~Pipeline() {
  gbuffer_framebuffer = hRenderTarget();
  default_framebuffer = hRenderTarget();
  current_framebuffer = hRenderTarget();
  default_texture = hTexture();
  loader_be_done = true;
  loader_thread.join();
  platformFinish();
}

void Pipeline::setupRenderTargets() {
  if(!default_framebuffer || !gbuffer_framebuffer)
    return;
  default_framebuffer->bound_textures.clear();
  gbuffer_framebuffer->bound_textures.clear();

  hTexture main_plane = createTargetTexture();
  hTexture depth = createTargetTexture();
  default_framebuffer->bound_textures.push_back(depth);
  default_framebuffer->bound_textures.push_back(main_plane);
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer->gl_fboid))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, main_plane->gl_texid))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cur_fsaa, GL_RGBA16F, width, height, GL_FALSE))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth->gl_texid))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cur_fsaa, GL_DEPTH24_STENCIL8, width, height, GL_FALSE))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, main_plane->gl_texid, 0))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth->gl_texid, 0))
  GL_CHECK(glViewport(0, 0, width, height))
  FBO_CHECK

  hTexture rgbm = createTargetTexture();
  hTexture nor = createTargetTexture();
  gbuffer_framebuffer->bound_textures.push_back(depth);
  gbuffer_framebuffer->bound_textures.push_back(rgbm);
  gbuffer_framebuffer->bound_textures.push_back(nor);
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_framebuffer->gl_fboid))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rgbm->gl_texid))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cur_fsaa, GL_RGBA8, width, height, GL_FALSE))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, nor->gl_texid))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cur_fsaa, GL_RG16F, width, height, GL_FALSE))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth->gl_texid, 0))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, rgbm->gl_texid, 0))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, nor->gl_texid, 0))
  GL_CHECK(glViewport(0, 0, width, height))
  FBO_CHECK
}

void Pipeline::pushKbdCallback(Pipeline::KbdCallback* )
{}

void Pipeline::beginFrame() {
  // TODO: anything that needs to happen at the beginning of a frame
}

void Pipeline::render() {
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_framebuffer->gl_fboid))
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
  // TODO: loop through objects and fill the gbuffer
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, current_framebuffer->gl_fboid)) // our current render target
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT))
  // TODO: loop through lights and fill the framebuffer
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0))
  if(current_framebuffer->build_mips) {
    for(auto i = current_framebuffer->bound_textures.begin(); i != current_framebuffer->bound_textures.end(); ++i) {
      GL_CHECK(glBindTexture(GL_TEXTURE_2D, (*i)->gl_texid))
      GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D))
    }
  }
}

void Pipeline::endFrame() {
  GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, default_framebuffer->gl_fboid))
  // TODO: convert this blit into a tonemapping operation of some sort
  GL_CHECK(glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST))
  platformSwap();
}

void Pipeline::setRenderTarget(hRenderTarget target) {
  if(!target) target = default_framebuffer;
  if(target == current_framebuffer) return;
  current_framebuffer = target;
}

Pipeline::hRenderTarget Pipeline::buildRenderTarget(int w, int h, bool mip) {
  hRenderTarget rt = createRenderTarget();
  hTexture tex = createTargetTexture();
  rt->bound_textures.push_back(tex);
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex->gl_texid))
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, 0))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
  if(mip) {
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR))
  } else {
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
  }
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, rt->gl_fboid))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->gl_texid, 0))
  FBO_CHECK
  return rt;
}

Pipeline::GpuMemAvailable Pipeline::queryAvailableMem() {
  GpuMemAvailable meminfo;
  if(GLEW_NVX_gpu_memory_info) {
    int mem;
    GL_CHECK(glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &mem))
    meminfo.buffer = mem;
    meminfo.render = mem;
    meminfo.texture = mem;
  } else if(GLEW_ATI_meminfo) {
    int data[4];
    GL_CHECK(glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, data))
    meminfo.buffer = data[0];
    GL_CHECK(glGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, data))
    meminfo.render = data[0];
    GL_CHECK(glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, data))
    meminfo.texture = data[0];
  } else {
    meminfo.buffer = -1;
    meminfo.render = -1;
    meminfo.texture = -1;
  }
  return meminfo;
}

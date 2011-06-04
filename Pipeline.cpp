#include "Pipeline.hpp"
#include <GL/glew.h>

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <cstdio>

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

#define STRING(X) #X
#define TOSTRING(X) STRING(X)
#define FILE_LINE " @ " __FILE__ ":" TOSTRING(__LINE__)
#ifndef NDEBUG
#define GL_CHECK(func) func; { GLenum glerr = glGetError(); if(GL_NO_ERROR != glerr) throw gl_error(FILE_LINE, glerr); }
#define FBO_CHECK { GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); if(GL_FRAMEBUFFER_COMPLETE != status) throw fbo_error(FILE_LINE, status); }
#else
#define GL_CHECK(func) func;
#define FBO_CHECK
#endif

void launchLoaderThread(Pipeline *pipe) {
  pipe->runLoaderThread();
}

struct TextureDealloc {
  Pipeline *pipe;
  TextureDealloc(Pipeline *pipe) : pipe(pipe) {}
  void operator()(Pipeline::Texture *p) { pipe->deleteTexture(p); }  
};

struct TargetTextureDealloc {
  Pipeline *pipe;
  TargetTextureDealloc(Pipeline *pipe) : pipe(pipe) {}
  void operator()(Pipeline::Texture *p) { pipe->deleteTargetTexture(p); }
};

struct RenderTargetDealloc {
  Pipeline *pipe;
  RenderTargetDealloc(Pipeline *pipe) : pipe(pipe) {}
  void operator()(Pipeline::RenderTarget *p) { pipe->deleteRenderTarget(p); }
};

void Pipeline::runLoaderThread() {
  try {
  platformInitLoader();
  } catch (std::exception& e) {
    loader_error_string = e.what();
  }
  loader_init_complete = true;

  for(;;) {
    // TODO: add loader job logic
  }
}

void Pipeline::deleteTexture(Texture *p) {
  // TODO: push a TextureDelete message to the loader thread
  delete p;
}

void Pipeline::deleteTargetTexture(Pipeline::Texture* p) {
  glDeleteTextures(1, (GLuint*)&(p->id));
  delete p;
}

void Pipeline::deleteRenderTarget(Pipeline::RenderTarget* p) {
  glDeleteFramebuffers(1, (GLuint*)&(p->id));
  delete p;
}

Pipeline::hTexture Pipeline::createTargetTexture() {
  hTexture tex(new Texture, TargetTextureDealloc(this));
  GLuint texid;
  glGenTextures(1, &texid);
  tex->id = texid;
  return tex;
}

Pipeline::hRenderTarget Pipeline::createRenderTarget() {
  hRenderTarget target(new RenderTarget, RenderTargetDealloc(this));
  GLuint id;
  glGenFramebuffers(1, &id);
  target->id = id;
  return target;
}

Pipeline::Pipeline() : loader_init_complete(false) {
  platformInit();
  glewExperimental = GL_TRUE;
  glewInit();

  platformDetachContext(); // On X11, the context can't be current when we setup the loader thread
  loader_thread = std::thread(std::bind(launchLoaderThread, this));
  while(!loader_init_complete);
  platformAttachContext();
  if(!loader_error_string.empty()) {
    platformFinish();
    throw std::runtime_error(loader_error_string.c_str());
  }

  int dsamples, csamples;
  glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &dsamples);
  glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &csamples);
  max_fsaa = std::min(dsamples, csamples);
  fsaa_count = 4; // A nice value for testing

  // Enable some standard state
  GL_CHECK(glEnable(GL_MULTISAMPLE))

  // Create the HDR lighting buffer
  hRenderTarget main_framebuffer = createRenderTarget();
  hTexture main_plane = createTargetTexture();
  hTexture depth = createTargetTexture();
  main_framebuffer->bound_textures.push_back(depth);
  main_framebuffer->bound_textures.push_back(main_plane);
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, main_framebuffer->id))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, main_plane->id))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fsaa_count, GL_RGBA16F, 800, 600, GL_FALSE))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth->id))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fsaa_count, GL_DEPTH24_STENCIL8, 800, 600, GL_FALSE))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, main_plane->id, 0))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth->id, 0))
  FBO_CHECK
  current_framebuffer = default_framebuffer = main_framebuffer;

  // Create the gbuffer
  hRenderTarget gbuf_framebuffer = createRenderTarget();
  hTexture rgbm = createTargetTexture();
  hTexture nor = createTargetTexture();
  gbuf_framebuffer->bound_textures.push_back(depth);
  gbuf_framebuffer->bound_textures.push_back(rgbm);
  gbuf_framebuffer->bound_textures.push_back(nor);
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, gbuf_framebuffer->id))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rgbm->id))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fsaa_count, GL_RGBA8, 800, 600, GL_FALSE))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, nor->id))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fsaa_count, GL_RG16F, 800, 600, GL_FALSE))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth->id, 0))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, rgbm->id, 0))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, nor->id, 0))
  FBO_CHECK
  gbuffer_framebuffer = gbuf_framebuffer;
  glClearColor(1.f, 0.f, 0.f, 1.f);

  GpuMemAvailable mem = queryAvailableMem();
  printf("Current FSAA setting:\t%d\n", fsaa_count);
  printf("Available texture mem:\t%d\nAvailable render mem:\t%d\nAvailable buffer mem:\t%d\n", mem.texture, mem.render, mem.buffer);
}

Pipeline::~Pipeline() {
  platformFinish();
}

void Pipeline::pushKbdCallback(Pipeline::KbdCallback* )
{}

void Pipeline::beginFrame() {
  // TODO: anything that needs to happen at the beginning of a frame
}

void Pipeline::render() {
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_framebuffer->id))
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
  // loop through objects and fill the gbuffer
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, current_framebuffer->id)) // our current render target
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT))
  // loop through lights and fill the framebuffer
}

void Pipeline::endFrame() {
  GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0))
  // TODO: convert this blit into a tonemapping operation of some sort
  GL_CHECK(glBlitFramebuffer(0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT, GL_NEAREST))
  platformSwap();
}

void Pipeline::setRenderTarget(hRenderTarget target) {
  if(!target) target = default_framebuffer;
  if(target == current_framebuffer) return;
  current_framebuffer = target;
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

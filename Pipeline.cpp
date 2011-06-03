#include "Pipeline.hpp"
#include <GL/glew.h>

#include <stdexcept>
#include <cstdlib>
#include <cstdio>

void launchLoaderThread(Pipeline *pipe) {
  pipe->runLoaderThread();
}

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

  // TODO: store this information someplace useful; move this to a function 
  if(GLEW_NVX_gpu_memory_info) {
    int mem;
    glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &mem);
    printf("Video Memory: %dk\n", mem);
  } else if(GLEW_ATI_meminfo) {
    printf("ATI_meminfo is unimplemented; assuming 256MB");
    // I'm not even sure how to parse ATI_meminfo correctly. Need to do science on it!
  } else {
    printf("I can't get the GPU memory size from this card; assuming 256MB");
  }
}

Pipeline::~Pipeline() {
  platformFinish();
}

void Pipeline::pushKbdCallback(Pipeline::KbdCallback* )
{}

void Pipeline::beginFrame() {
  int col = rand();
  uint8_t r, g, b;
  r = col & 0xFF;
  g = (col >> 8) & 0xFF;
  b = (col >> 16) & 0xFF;
  glClearColor(float(r)/255.f, float(g)/255.f, float(b)/255.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Pipeline::endFrame() {
  platformSwap();
}
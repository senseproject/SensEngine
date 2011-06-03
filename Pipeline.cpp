#include "Pipeline.hpp"
#include <GL/glew.h>

#include <stdexcept>
#include <cstdlib>

void launchLoaderThread(Pipeline *pipe) {
  pipe->runLoaderThread();
}

void Pipeline::runLoaderThread() {
  try {
  platformInitLoader();
  } catch (std::exception& e) {
    loader_init_error = true;
    loader_error_string = e.what();
  }
  loader_init_complete = true;

  for(;;) {
    // TODO: add loader job logic
  }
}

Pipeline::Pipeline() : loader_init_complete(false), loader_init_error(false) {
  platformInit();
  glewInit();

  platformDetachContext(); // On X11, the context can't be current when we setup the loader thread
  loader_thread = std::thread(std::bind(launchLoaderThread, this));
  while(!loader_init_complete);
  platformAttachContext();
  if(loader_init_error) {
    platformFinish();
    throw std::runtime_error(loader_error_string.c_str());
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
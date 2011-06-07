#include "RenderTarget.hpp"
#include "GL/glew.h"

RenderTarget::RenderTarget() {
  glGenFramebuffers(1, &gl_fboid);
}

RenderTarget::~RenderTarget() {
  glDeleteFramebuffers(1, &gl_fboid);
}
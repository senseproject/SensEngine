#include "Texture.hpp"

#include "GL/glew.h"

Texture::Texture() {
  glGenTextures(1, &gl_texid);
}

Texture::~Texture() {
  glDeleteTextures(1, &gl_texid);
}

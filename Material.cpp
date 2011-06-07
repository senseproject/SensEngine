#include "GL/glew.h"
#include "Material.hpp"

GlShader::~GlShader() {
  if(gl_id) glDeleteShader(gl_id);
}

ShaderProgram::~ShaderProgram() {
  if(gl_id) glDeleteProgram(gl_id);
}

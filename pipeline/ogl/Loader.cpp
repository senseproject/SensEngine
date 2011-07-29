// Copyright 2011 Branan Purvine-Riley and Adam Johnson
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "python/module.hpp"

#include "implementation.hpp"
#include "../interface.hpp"
#include "../Drawable.hpp"
#include "../Builtins.hpp"
#include "glexcept.hpp"
// #include "Webview.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sstream>

Texture::~Texture()
{}

Loader::Loader()
  : self(new LoaderImpl)
{
  self->finished = false;

  std::stringstream ss;
  unsigned int num_instances = 64; // TODO: fetch from config system
  ss << "#version 150" << std::endl;
//  ss << "#extension GL_ARB_explicit_attrib_location : require" << std::endl;
  ss << "#define SENSE_MAX_INSTANCES " << num_instances << std::endl;
/*  ss << "#define SENSE_VERT_INPUT_POS " << DrawableMesh::Pos << std::endl;
  ss << "#define SENSE_VERT_INPUT_NOR " << DrawableMesh::Nor << std::endl;
  ss << "#define SENSE_VERT_INPUT_TAN " << DrawableMesh::Tan << std::endl;
  ss << "#define SENSE_VERT_INPUT_COL " << DrawableMesh::Col << std::endl;
  ss << "#define SENSE_VERT_INPUT_TE0 " << DrawableMesh::Te0 << std::endl;
  ss << "#define SENSE_VERT_INPUT_TE1 " << DrawableMesh::Te1 << std::endl;
  ss << "#define SENSE_VERT_INPUT_SKINIDX " << DrawableMesh::SkinIdx << std::endl;
  ss << "#define SENSE_VERT_INPUT_SKINWEIGHT " << DrawableMesh::SkinWeight << std::endl;
  ss << "#define SENSE_FRAG_OUTPUT_COL 0" << std::endl;
  ss << "#define SENSE_FRAG_OUTPUT_NOR 1" << std::endl; */
  ss << std::endl;
  self->shader_header = ss.str();
}

Loader::~Loader()
{}

GlShader* LoaderImpl::loadShader(std::string shader_source, GLenum gl_shader_type) {
  if(shader_source.empty())
    return NULL;

  auto it = shaders.find(shader_source);
  if(it != shaders.end()) {
    it->second->refcnt++;
    return it->second;
  }

  GlShader* shader = new GlShader;
  shader->refcnt = 1;
  shader->gl_id = GL_CHECK(glCreateShader(gl_shader_type))
  const char* sources[2];
  sources[0] = shader_header.c_str();
  sources[1] = shader_source.c_str();
  GL_CHECK(glShaderSource(shader->gl_id, 2, sources, 0))
  GL_CHECK(glCompileShader(shader->gl_id))
  int compile_status;
  GL_CHECK(glGetShaderiv(shader->gl_id, GL_COMPILE_STATUS, &compile_status))
  if(compile_status == GL_FALSE) {
    int info_log_length;
    GL_CHECK(glGetShaderiv(shader->gl_id, GL_INFO_LOG_LENGTH, &info_log_length))
    char *log = new char[info_log_length];
    GL_CHECK(glGetShaderInfoLog(shader->gl_id, info_log_length, &info_log_length, log))
    std::string infolog = log;
    delete[] log;
    throw std::runtime_error("Error compiling shader:\n" + infolog);
  }
  
  return shader;
}

ShaderProgram* Loader::loadProgram(std::string vert, std::string frag, std::string geom) {
  ShaderSet s;
  s.vert = self->loadShader(vert, GL_VERTEX_SHADER);
  s.frag = self->loadShader(frag, GL_FRAGMENT_SHADER);
  s.geom = self->loadShader(geom, GL_GEOMETRY_SHADER);

  auto it = self->programs.find(s);
  if(it != self->programs.end()) {
    ShaderProgram* prog = it->second;
    prog->refcnt++;
    return prog;
  }

  ShaderProgram* prog = new ShaderProgram;
  prog->refcnt = 1;
  prog->gl_id = GL_CHECK(glCreateProgram());
  prog->vert = s.vert;
  prog->geom = s.geom;
  prog->frag = s.frag;
  if(s.vert)
    s.vert->refcnt++;
  else
    throw std::runtime_error("OpenGL Programs must have at least a vertex and fragment shader (missing fragment)");
  if(s.frag)
    s.frag->refcnt++;
  else
    throw std::runtime_error("OpenGL Programs must have at least a vertex and fragment shader (missing vertex)");
  if(s.geom)
    s.geom->refcnt++;
  prog->vert = s.vert;
  prog->geom = s.geom;
  prog->frag = s.frag;
  GL_CHECK(glAttachShader(prog->gl_id, prog->vert->gl_id))
  if (prog->geom) {
    GL_CHECK(glAttachShader(prog->gl_id, prog->geom->gl_id))
  }
  GL_CHECK(glAttachShader(prog->gl_id, prog->frag->gl_id))

  GL_CHECK(glBindFragDataLocation(prog->gl_id, 0, "fcol"))
  GL_CHECK(glBindFragDataLocation(prog->gl_id, 1, "fnor"))
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Pos, "pos"))
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Nor, "nor"))
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Tan, "tan"))
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Col, "col"))
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Te0, "te0"))
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Te1, "te1"))
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::SkinIdx, "ski"))
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::SkinWeight, "skw"))

  GL_CHECK(glLinkProgram(prog->gl_id))
  int link_status;
  GL_CHECK(glGetProgramiv(prog->gl_id, GL_LINK_STATUS, &link_status))
  if(link_status == GL_FALSE) {
    int info_log_length;
    GL_CHECK(glGetProgramiv(prog->gl_id, GL_INFO_LOG_LENGTH, &info_log_length))
    char *log = new char[info_log_length];
    GL_CHECK(glGetProgramInfoLog(prog->gl_id, info_log_length, &info_log_length, log))
    std::string infolog = log;
    delete[] log;
    throw std::runtime_error("Error linking program: \n" + infolog);
  }
  self->programs.insert(std::make_pair(s, prog));
  return prog;
}

bool Loader::isThreaded() {
  return true;
}

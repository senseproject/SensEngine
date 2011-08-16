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
#include "../Image.hpp"
#include "glexcept.hpp"
// #include "Webview.hpp"

#include <sstream>

Loader::Loader()
  : self(new LoaderImpl)
{
  std::stringstream ss;
  ss << "#version 150" << std::endl;
  ss << "#define SENSE_MAX_INSTANCES " << SENSE_MAX_INSTANCES << std::endl;
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
  shader->gl_id = GL_CHECK(glCreateShader(gl_shader_type));
  const char* sources[2];
  sources[0] = shader_header.c_str();
  sources[1] = shader_source.c_str();
  GL_CHECK(glShaderSource(shader->gl_id, 2, sources, 0));
  GL_CHECK(glCompileShader(shader->gl_id));
  int compile_status;
  GL_CHECK(glGetShaderiv(shader->gl_id, GL_COMPILE_STATUS, &compile_status));
  if(compile_status == GL_FALSE) {
    int info_log_length;
    GL_CHECK(glGetShaderiv(shader->gl_id, GL_INFO_LOG_LENGTH, &info_log_length));
    char *log = new char[info_log_length];
    GL_CHECK(glGetShaderInfoLog(shader->gl_id, info_log_length, &info_log_length, log));
    std::string infolog = log;
    delete[] log;
    throw std::runtime_error("Error compiling shader:\n" + infolog);
  }
  
  return shader;
}

void Loader::loadMesh(DrawableMesh* m)
{
  GLuint vtx_buf, idx_buf=0;
  GLenum idx_type = 0;
  GL_CHECK(glGenBuffers(1, &vtx_buf));
  GL_CHECK(glBindBuffer(GL_COPY_WRITE_BUFFER, vtx_buf));
  GL_CHECK(glBufferData(GL_COPY_WRITE_BUFFER, m->data_size, m->data, GL_STATIC_DRAW));

  if(m->index_data) {
    GLuint idx_stride;
    switch(m->index_type) {
    case DrawableMesh::UByte: idx_stride = 1; idx_type = GL_UNSIGNED_BYTE; break;
    case DrawableMesh::UShort: idx_stride = 2; idx_type = GL_UNSIGNED_SHORT; break;
    default: idx_stride = 0; break;
    }

    GL_CHECK(glGenBuffers(1, &idx_buf));
    GL_CHECK(glBindBuffer(GL_COPY_WRITE_BUFFER, idx_buf));
    GL_CHECK(glBufferData(GL_COPY_WRITE_BUFFER, m->index_count*idx_stride, m->index_data, GL_STATIC_DRAW));
  }

  // make sure the data is actually ready before giving it to the main thread
  GL_CHECK(glFinish());

  DrawableBuffer* b = new DrawableBuffer;
  b->vao = 0;
  b->vtxbuffer = vtx_buf;
  b->idxbuffer = idx_buf;
  b->idx_type = idx_type;

  m->buffer = b;
}

void Loader::mainThreadLoadMesh(DrawableMesh* m)
{
  GLuint vao;
  GL_CHECK(glGenVertexArrays(1, &vao));
  GL_CHECK(glBindVertexArray(vao));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m->buffer->vtxbuffer));
  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->buffer->idxbuffer));

  std::set<DrawableMesh::AttribLocation> used_locs;
  auto end = m->attributes.end();
  for(auto i = m->attributes.begin(); i != end; ++i) {
    GLenum type = 0;
    switch(i->type) {
    case DrawableMesh::Byte: type = GL_BYTE;
      break;
    case DrawableMesh::UByte: type = GL_UNSIGNED_BYTE;
      break;
    case DrawableMesh::Short: type = GL_SHORT;
      break;
    case DrawableMesh::UShort: type = GL_UNSIGNED_SHORT;
      break;
    case DrawableMesh::Int: type = GL_INT;
      break;
    case DrawableMesh::UInt: type = GL_UNSIGNED_INT;
      break;
    case DrawableMesh::Float: type = GL_FLOAT;
      break;
    case DrawableMesh::Half: type = GL_HALF_FLOAT;
      break;
    case DrawableMesh::Double: type = GL_DOUBLE;
      break;
    }

    GL_CHECK(glEnableVertexAttribArray(i->loc));
    if(i->special == DrawableMesh::Integer) {
      GL_CHECK(glVertexAttribIPointer(i->loc, i->size, type, m->data_stride, (GLvoid*)(i->start)));
    } else {
      GLboolean normalized = (i->special == DrawableMesh::Normalize) ? GL_TRUE : GL_FALSE;
      GL_CHECK(glVertexAttribPointer(i->loc, i->size, type, normalized, m->data_stride, (GLvoid*)(i->start)));
    }
    used_locs.insert(i->loc);
  }

  // Set sane default values for all parameters that aren't defined in this mesh
  if(!used_locs.count(DrawableMesh::Pos))
    GL_CHECK(glVertexAttrib3f(DrawableMesh::Pos, 0.f, 0.f, 0.f));
  if(!used_locs.count(DrawableMesh::Nor))
    GL_CHECK(glVertexAttrib3f(DrawableMesh::Nor, 0.f, 0.f, 1.f));
  if(!used_locs.count(DrawableMesh::Tan))
    GL_CHECK(glVertexAttrib3f(DrawableMesh::Tan, 0.f, 1.f, 0.f));
  if(!used_locs.count(DrawableMesh::Col))
    GL_CHECK(glVertexAttrib3f(DrawableMesh::Col, 0.f, 0.f, 0.f));
  if(!used_locs.count(DrawableMesh::Te0))
    GL_CHECK(glVertexAttrib2f(DrawableMesh::Te0, 0.f, 0.f));
  if(!used_locs.count(DrawableMesh::Te1))
    GL_CHECK(glVertexAttrib2f(DrawableMesh::Te1, 0.f, 0.f));
  if(!used_locs.count(DrawableMesh::SkinIdx))
    GL_CHECK(glVertexAttrib4s(DrawableMesh::SkinIdx, 0, 0, 0, 0));
  if(!used_locs.count(DrawableMesh::SkinWeight))
    GL_CHECK(glVertexAttrib4f(DrawableMesh::SkinWeight, 0.f, 0.f, 0.f, 0.f));

  m->buffer->vao = vao;
}

boost::any Loader::queryUniform(ShaderProgram* prog, std::string name)
{
  return boost::any(glGetUniformLocation(prog->gl_id, name.c_str()));
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
  GL_CHECK(glAttachShader(prog->gl_id, prog->vert->gl_id));
  if (prog->geom) {
    GL_CHECK(glAttachShader(prog->gl_id, prog->geom->gl_id));
  }
  GL_CHECK(glAttachShader(prog->gl_id, prog->frag->gl_id));

  GL_CHECK(glBindFragDataLocation(prog->gl_id, 0, "fcol"));
  GL_CHECK(glBindFragDataLocation(prog->gl_id, 1, "fnor"));
  GL_CHECK(glBindFragDataLocation(prog->gl_id, 2, "fmat"));
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Pos, "pos"));
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Tan, "tan"));
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Col, "col"));
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Te0, "te0"));
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::Te1, "te1"));
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::SkinIdx, "ski"));
  GL_CHECK(glBindAttribLocation(prog->gl_id, DrawableMesh::SkinWeight, "skw"));

  GL_CHECK(glLinkProgram(prog->gl_id));

  int link_status;
  GL_CHECK(glGetProgramiv(prog->gl_id, GL_LINK_STATUS, &link_status));
  if(link_status == GL_FALSE) {
    int info_log_length;
    GL_CHECK(glGetProgramiv(prog->gl_id, GL_INFO_LOG_LENGTH, &info_log_length));
    char *log = new char[info_log_length];
    GL_CHECK(glGetProgramInfoLog(prog->gl_id, info_log_length, &info_log_length, log));
    std::string infolog = log;
    delete[] log;
    throw std::runtime_error("Error linking program: \n" + infolog);
  }
  self->programs.insert(std::make_pair(s, prog));
  return prog;
}

void Loader::releaseProgram(ShaderProgram* program)
{
  if(program)
    program->refcnt--;
  // No actual resource freeing is done yet. Garbage collection is handled when needed.
}

void Loader::loadTexture(Image* img) {
  GLenum internal_format, format;
  GLuint texid;
  switch(img->format) {
  case Image::R8: internal_format=GL_R8; format=GL_R; break;
  case Image::RG8: internal_format=GL_RG8; format=GL_RG; break;
  case Image::RGB8: internal_format=GL_RGB8; format=GL_RGB; break;
  case Image::RGBA8: internal_format=GL_RGBA8; format=GL_RGBA; break;
  }

  GL_CHECK(glGenTextures(1, &texid));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, texid));
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img->width, img->height, 0, format, GL_UNSIGNED_BYTE, img->data));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  if(img->pipe_build_mips) {
    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
  }
  GL_CHECK(glFinish());
  img->tex = new Texture;
  img->tex->id = texid;
}

bool Loader::isThreaded() {
  return true;
}

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

#include <python/module.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "Loader.hpp"
#include "Builtins.hpp"
#include "Drawbuffer.hpp"
#include "Material.hpp"
#include "Webview.hpp"
#include "GL/glew.h"
#include "glexcept.hpp"
#include "util/util.hpp"

void Loader::init() {
  std::stringstream ss;
  unsigned int num_instances = 64; // TODO: fetch from config system
  ss << "#version 150" << std::endl;
  ss << "#extension GL_ARB_explicit_attrib_location : require" << std::endl;
  ss << "#define SENSE_MAX_INSTANCES " << num_instances << std::endl;
  ss << "#define SENSE_VERT_INPUT_POS " << DrawBuffer::Pos << std::endl;
  ss << "#define SENSE_VERT_INPUT_NOR " << DrawBuffer::Nor << std::endl;
  ss << "#define SENSE_VERT_INPUT_TAN " << DrawBuffer::Tan << std::endl;
  ss << "#define SENSE_VERT_INPUT_COL " << DrawBuffer::Col << std::endl;
  ss << "#define SENSE_VERT_INPUT_TE0 " << DrawBuffer::Te0 << std::endl;
  ss << "#define SENSE_VERT_INPUT_TE1 " << DrawBuffer::Te1 << std::endl;
  ss << "#define SENSE_VERT_INPUT_SKINIDX " << DrawBuffer::SkinIdx << std::endl;
  ss << "#define SENSE_VERT_INPUT_SKINWEIGHT " << DrawBuffer::SkinWeight << std::endl;
  ss << "#define SENSE_FRAG_OUTPUT_COL 0" << std::endl;
  ss << "#define SENSE_FRAG_OUTPUT_NOR 1" << std::endl;
  ss << std::endl;
  shader_header = ss.str();

  MaterialDef m;
  m.shaders.frag = "guiview"; // maybe we should rename this shader to something more general :P
  m.shaders.vert = "guiview";
  UniformDef u;
  u.type = UniformDef::ModelView;
  m.uniforms.insert(std::make_pair("modelview", u));
  u.type = UniformDef::Projection;
  m.uniforms.insert(std::make_pair("projection", u));
  u.type = UniformDef::Texture;
  u.value = std::string("__MISSING__");
  m.uniforms.insert(std::make_pair("guitex", u));
  material_defs.insert(std::make_pair("__MISSING__", m));
}

void Loader::run() {
}

void Loader::cleanup() {
}

std::shared_ptr<Material> Loader::loadMaterial(std::string material) {
  auto it = materials.find(material);
  if(it != materials.end() && !it->second.expired())
    return it->second.lock();

  auto i = material_defs.find(material);
  std::shared_ptr<Material> mat;
  if(i != material_defs.end()) {
    mat = std::shared_ptr<Material>(new Material);
    buildMaterial(mat, *i);
    materials.insert(std::make_pair(material, mat));
  }
  if(!mat)
   mat = loadMaterial("__MISSING__");
  return mat;
}

std::shared_ptr<DrawBuffer> Loader::loadMesh(std::string model) {
  auto it = meshes.find(model);
  if(it != meshes.end() && !it->second.expired())
    return it->second.lock();
  if(model == "__quad__") {
    DrawBuffer *buf = new DrawBuffer;
    buf->bind();
    buf->setData(builtin_quad_data, 6, 20);
    buf->attribute(DrawBuffer::Pos, 3, 0, DrawBuffer::Float);
    buf->attribute(DrawBuffer::Te0, 2, 12, DrawBuffer::Float);
    std::shared_ptr<DrawBuffer> b(buf);
    meshes.insert(std::make_pair(model, b));
    return b;
  } else if (model == "__MISSING__") {
    IndexedDrawBuffer *buf = new IndexedDrawBuffer;
    buf->bind();
    buf->setData(builtin_missing_data, 24, 20);
    buf->attribute(DrawBuffer::Pos, 3, 0, DrawBuffer::Float);
    buf->attribute(DrawBuffer::Te0, 2, 12, DrawBuffer::Float);
    buf->setIndices(builtin_missing_indices, builtin_missing_idx_count, DrawBuffer::UShort);
    std::shared_ptr<DrawBuffer> b(buf);
    meshes.insert(std::make_pair(model, b));
    return b;
  } else
    return loadMesh("__MISSING__");
}

void Loader::addMaterial(std::string name, MaterialDef def) {
  auto i = material_defs.insert(std::make_pair(name, def));
  auto it = materials.find(name);
  if(it != materials.end() && !it->second.expired())
      buildMaterial(it->second.lock(), *(i.first));
}

void Loader::loadMaterialFiles(boost::filesystem::path dir) {
  if(!exists(dir)) return;
  PyObject* old_path = appendSysPath(dir.string().c_str(), true);

  boost::filesystem::directory_iterator end_itr;
  for(boost::filesystem::directory_iterator itr(dir); itr != end_itr; ++itr) {
    if(itr->path().extension() == ".smtl") {
      boost::filesystem::ifstream stream;
      stream.open(itr->path());
      std::string pymtlcode((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
      PyRun_SimpleString(pymtlcode.c_str());
    }
  }
  restoreSysPath(old_path);
}


void Loader::buildMaterial(std::shared_ptr<Material> mat, std::pair<std::string, MaterialDef> matdef) {
  mat->program = loadProgram(matdef.second.shaders);
  GL_CHECK(glUseProgram(mat->program->gl_id))
  for(auto i = matdef.second.uniforms.begin(); i != matdef.second.uniforms.end(); ++i) {
    Uniform u;
    u.gl_id = glGetUniformLocation(mat->program->gl_id, i->first.c_str());
    if(u.gl_id == -1)
      continue;
    u.type = i->second.type;
    switch(u.type) {
    case UniformDef::Texture:
      u.value = loadTexture(boost::any_cast<std::string>(i->second.value));
      break;
    case UniformDef::Webview:
      u.type = UniformDef::Texture; // Webviews are a special class of texture, but the Pipeline doesn't need to know that!
      u.value = loadWebview(boost::any_cast<std::string>(i->second.value));
      break;
    default:
      break;
    }
    mat->uniforms.push_back(u);
  }
}

std::shared_ptr<Texture> Loader::loadTexture(std::string path) {
  auto it = textures.find(path);
  if(it != textures.end() && !it->second.expired())
    return it->second.lock();
  if(path == "__MISSING__") {
    Texture *t = new Texture;
    t->name = path;
    t->hres = 2;
    t->vres = 2;
    t->biggest_mip_loaded = 0;
    glBindTexture(GL_TEXTURE_2D, t->gl_texid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, builtin_missingtex_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    std::shared_ptr<Texture> tex(t);
    textures.insert(std::make_pair(path, tex));
    return tex;
  }

  return loadTexture("__MISSING__");
}

std::shared_ptr<Texture> Loader::loadWebview(std::string path) {
  boost::filesystem::path html_path = boost::filesystem::current_path() / "../data/html" / path;
  Webview* w = new Webview("file://"+html_path.string(), 800, 600);
  return std::shared_ptr<Texture>(w);
}

std::shared_ptr<GlShader> Loader::loadShader(std::string path, ShaderType type) {
  if(path.empty())
    return std::shared_ptr<GlShader>();

  auto it = shaders.find(path);
  if(it != shaders.end() && !it->second.expired())
    return it->second.lock();

  GLenum gl_shader_type;
  switch(type) {
    case Vertex: gl_shader_type = GL_VERTEX_SHADER; break;
    case Fragment: gl_shader_type = GL_FRAGMENT_SHADER; break;
    case Geometry: gl_shader_type = GL_GEOMETRY_SHADER; break;
  }

  GlShader* shader = new GlShader;
  shader->gl_id = GL_CHECK(glCreateShader(gl_shader_type))
  boost::filesystem::ifstream stream;
  stream.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
  boost::filesystem::path shader_path("../data/shaders");
  shader_path = shader_path / path;
  if(!exists(shader_path))
    throw std::runtime_error("Can't find shader file "+shader_path.string());
  stream.open(shader_path);
  std::string shader_source((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
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
    throw std::runtime_error("Error compiling shader " + shader_path.string() + ":\n" + infolog);
  }
  
  return std::shared_ptr<GlShader>(shader);
}

std::shared_ptr<ShaderProgram> Loader::loadProgram(ShaderKey key) {
  auto it = programs.find(key);
  if(it != programs.end() && !it->second.expired())
    return it->second.lock();
  std::shared_ptr<ShaderProgram> prog = std::shared_ptr<ShaderProgram>(new ShaderProgram);
  prog->gl_id = GL_CHECK(glCreateProgram())
  if(!key.frag.empty())
    prog->frag = loadShader(key.frag+".fs", Fragment);
  else
    throw std::runtime_error("Programs must have at least a vertex and fragment shader (missing fragment)");
  if(!key.vert.empty())
    prog->vert = loadShader(key.vert+".vs", Vertex);
  else
    throw std::runtime_error("Programs must have at least a vertex and fragment shader (missing vertex)");
  if(!key.geom.empty())
    prog->geom = loadShader(key.geom+".gs", Geometry);
  GL_CHECK(glAttachShader(prog->gl_id, prog->vert->gl_id))
  if(prog->geom)
    GL_CHECK(glAttachShader(prog->gl_id, prog->geom->gl_id))
  GL_CHECK(glAttachShader(prog->gl_id, prog->frag->gl_id))
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
  programs.insert(std::make_pair(key, prog));
  return prog;
}

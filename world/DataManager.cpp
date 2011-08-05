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

#include "DataManager.hpp"
#include "Builtins.hpp"
#include "pipeline/Drawable.hpp"
#include "pipeline/Material.hpp"
#include "pipeline/interface.hpp"

#include <boost/thread/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

enum {
  BUILD_MATERIAL,
  FINISH_MESH_LOAD,
};

DataManager::DataManager(Loader* loader)
  : m_loader(loader)
{
  loadBuiltinData();
}

DataManager::~DataManager()
{}

void DataManager::exec()
{
  using boost::any_cast;

  m_finished = false;
  job j;
  while(!m_finished) {
    while(m_jobs.try_pop(j)) {
      switch(j.first) {
      case BUILD_MATERIAL:
        buildMaterial(any_cast<std::string>(j.second));
        break;
      }
    }
    boost::this_thread::yield();
  }
}

void DataManager::finish()
{
  m_finished = true;
}

void DataManager::mainThreadTick()
{
  using boost::any_cast;

  job j;
  while(m_main_thread_jobs.try_pop(j)) {
    switch(j.first) {
    case FINISH_MESH_LOAD: 
      m_loader->mainThreadLoadMesh(any_cast<DrawableMesh*>(j.second));
      break;
    }
  }
}

Material* DataManager::loadMaterial(std::string name)
{
  auto i = m_materials.find(name);
  if(i != m_materials.end()) {
    i->second->refcnt++;
    return i->second;
  } else {
    Material* m = new Material;
    m->shaders = 0;
    m->refcnt = 0;
    m_materials.insert(std::make_pair(name, m));
    m_jobs.push(job(BUILD_MATERIAL, name));
    return m;
  }
}

void DataManager::addMaterial(MaterialDef def, std::string name)
{
  bool inserted = m_matdefs.insert(std::make_pair(name, def)).second;
  if(!inserted) {
    m_matdefs[name] = def;
    if(m_materials.find(name) != m_materials.end()) {
      // there are instances of this material. Reload the sucker!
      m_jobs.push(job(BUILD_MATERIAL, name));
    }
  }
}

DrawableMesh* DataManager::loadMesh(std::string name)
{
  auto i = m_meshes.find(name);
  if(i != m_meshes.end()) {
    i->second->refcnt++;
    return i->second;
  }
  return NULL;
}

void DataManager::buildMaterial(std::string name)
{
  MaterialDef def = m_matdefs[name];
  std::string vert, frag, geom;
  vert = loadShaderString(def.shaders.vert + ".vs");
  frag = loadShaderString(def.shaders.frag + ".fs");
  geom = loadShaderString(def.shaders.geom + ".gs");

  ShaderProgram* s = m_loader->loadProgram(vert, frag, geom);
  std::vector<Uniform> uniforms;
  for(auto i = def.uniforms.begin(); i!= def.uniforms.end(); ++i) {
    Uniform u;
    u.type = i->second.type;
    u.value = i->second.value; // incorrect for texture uniforms, but works for all the simple types. FIXME
    u.pipe_id = m_loader->queryUniform(s, i->first);
    uniforms.push_back(u);
  }

  Material *m;
  auto i = m_materials.find(name);
  if(i != m_materials.end()) {
    m = i->second;
    m_loader->releaseProgram(m->shaders);
  } else {
    // This *should* never be a valid codepath. But just in case...
    m = new Material;
    m->refcnt = 0;
    m_materials.insert(std::make_pair(name, m));
  }
  m->uniforms = uniforms;
  m->shaders = s;
}

std::string DataManager::loadShaderString(std::string name)
{
  if(name.size() == 3 && name[0] == '.' && name[2] == 's' &&
     ( name[1] == 'f' || name[1] == 'v' || name[1] == 'g'))
    return ""; // empty shader name means empty shader string
  auto i = m_shaderstrings.find(name);
  if(i != m_shaderstrings.end())
    return i->second;

  boost::filesystem::path shader_path("../data/shaders");
  shader_path = shader_path / name;
  if(!exists(shader_path))
    throw std::runtime_error("Can't find shader file " + shader_path.string());
  boost::filesystem::ifstream stream;
  stream.open(shader_path);
  std::string shader =  std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
  m_shaderstrings.insert(std::make_pair(name, shader));
  return shader;
}

void DataManager::loadBuiltinData()
{
  DrawableMesh* builtin;
  DrawableMesh::Attribute a;

  a.type = DrawableMesh::Float;
  a.special = DrawableMesh::None;

  builtin = new DrawableMesh;
  builtin->refcnt = 1; // builtin data is *never* erased
  builtin->data = builtin_quad_data;
  builtin->data_size = 120;
  builtin->data_stride = 20;
  builtin->index_data = 0;

  a.loc = DrawableMesh::Pos;
  a.start = 0;
  a.size = 3;
  builtin->attributes.push_back(a);

  a.loc = DrawableMesh::Te0;
  a.start = 12;
  a.size = 2;
  builtin->attributes.push_back(a);

  m_meshes.insert(std::make_pair("__quad__", builtin));
  m_loader->loadMesh(builtin);
  m_main_thread_jobs.push(job(FINISH_MESH_LOAD, builtin));

  builtin = new DrawableMesh;
  builtin->refcnt = 1; // builtin data is *never* erased
  builtin->data = builtin_missing_data;
  builtin->data_size = 480;
  builtin->data_stride = 20;
  builtin->index_data = builtin_missing_indices;
  builtin->index_count = 36;
  builtin->index_type = DrawableMesh::UShort;

  a.loc = DrawableMesh::Pos;
  a.start = 0;
  a.size = 3;
  builtin->attributes.push_back(a);

  a.loc = DrawableMesh::Te0;
  a.start = 12;
  a.size = 2;
  builtin->attributes.push_back(a);

  m_meshes.insert(std::make_pair("__missing__", builtin));
  m_loader->loadMesh(builtin);
  m_main_thread_jobs.push(job(FINISH_MESH_LOAD, builtin));
}

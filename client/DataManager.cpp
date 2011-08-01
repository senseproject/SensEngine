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
#include "pipeline/Material.hpp"
#include "pipeline/interface.hpp"

#include <boost/thread/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

DataManager::DataManager(Loader* loader)
  : m_loader(loader)
{}

DataManager::~DataManager()
{}

void DataManager::exec()
{
  m_finished = false;
  while(!m_finished)
    boost::this_thread::yield();
}

void DataManager::finish()
{
  m_finished = true;
}

void DataManager::mainThreadTick()
{}

void DataManager::addMaterial(MaterialDef def, std::string name)
{
  bool inserted = m_matdefs.insert(std::make_pair(name, def)).second;
  if(!inserted) {
    m_matdefs[name] = def;
    buildMaterial(name);
  }
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
  }
  else {
    m = new Material;
    m->refcnt = 0;
    m_materials.insert(std::make_pair(name, m));
  }
  m->shaders = s;
  m->uniforms = uniforms;
}

std::string DataManager::loadShaderString(std::string name)
{
  boost::filesystem::path shader_path("../data/shaders");
  shader_path = shader_path / name;
  if(!exists(shader_path))
    throw std::runtime_error("Can't find shader file " + shader_path.string());
  boost::filesystem::ifstream stream;
  stream.open(shader_path);
  return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

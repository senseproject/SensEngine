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

#ifndef SENSE_CLIENT_DATAMANAGER_HPP
#define SENSE_CLIENT_DATAMANAGER_HPP

#include "pipeline/DefinitionTypes.hpp"

#include "util/queue.hpp"

#include <boost/thread/mutex.hpp>
#include <unordered_map>
#include <string>

class Loader;
struct Material;
struct DrawableMesh;
struct Image;

// The data manager loads anything that exists inside
// a game data package. It should be run on a worker thread,
// and "mainThreadTick" should be called every frame in the main rendering thread.
class DataManager
{
public:
  DataManager(Loader*);
  ~DataManager();

  void exec();
  void finish();
  void mainThreadTick();

  Material* loadMaterial(std::string);
  void addMaterial(MaterialDef, std::string);

  DrawableMesh* loadMesh(std::string);

private:
  Loader* m_loader;
  volatile bool m_finished;

  std::unordered_map<std::string, Material*> m_materials;
  std::unordered_map<std::string, MaterialDef> m_matdefs;
  std::unordered_map<std::string, std::string> m_shaderstrings;
  std::unordered_map<std::string, DrawableMesh*> m_meshes;
  std::unordered_map<std::string, Image*> m_images;

  boost::mutex m_imglock;

  void buildMaterial(std::string);
  std::string loadShaderString(std::string);

  void loadTexture(std::string);

  void loadMeshFile(std::string);

  typedef std::pair<unsigned int, boost::any> job;
  queue<job> m_jobs;
  queue<job> m_main_thread_jobs;

  void loadBuiltinData();
};


#endif // SENSE_CLIENT_DATAMANAGER_HPP

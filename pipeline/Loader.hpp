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

#ifndef SENSE_PIPELINE_LOADER_HPP
#define SENSE_PIPELINE_LOADER_HPP

#include <memory>
#include <string>
#include "DefinitionTypes.hpp"
#include <unordered_map>
#include <boost/filesystem/path.hpp>
#include <boost/thread/shared_mutex.hpp>

class GlShader;
class DrawBuffer;
class Material;
class ShaderProgram;
class Texture;

class Loader {
public:
  std::shared_ptr<Material> loadMaterial(std::string materia);
  std::shared_ptr<DrawBuffer> loadMesh(std::string model);

  void addMaterial(std::string name, MaterialDef def);
  void loadMaterialFiles(boost::filesystem::path);
private:
  std::string shader_header;
  void buildMaterial(std::shared_ptr<Material> mat, std::pair<std::string, MaterialDef> matdef);
  std::shared_ptr<Texture> loadTexture(std::string path);
  std::shared_ptr<Texture> loadWebview(std::string path);
  enum ShaderType {
    Vertex,
    Fragment,
    Geometry,
  };
  std::shared_ptr<GlShader> loadShader(std::string path, ShaderType type);
  std::shared_ptr<ShaderProgram> loadProgram(ShaderKey key);

  std::unordered_map<std::string, std::weak_ptr<Material>> materials;
  std::unordered_map<std::string, MaterialDef> material_defs;
  std::unordered_map<std::string, std::weak_ptr<DrawBuffer>> meshes;
  std::unordered_map<ShaderKey, std::weak_ptr<ShaderProgram>> programs;
  std::unordered_map<std::string, std::weak_ptr<GlShader>> shaders;
  std::unordered_map<std::string, std::weak_ptr<Texture>> textures;

  void init();
  void run();
  void cleanup();

public:
#ifndef _MSC_VER
  Loader()=default;
  ~Loader()=default;
#else
  Loader() {}
  ~Loader() {}
#endif
  friend class Pipeline;
};

#endif // SENSE_PIPELINE_LOADER_HPP

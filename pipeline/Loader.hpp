#ifndef SENSE_PIPELINE_LOADER_HPP
#define SENSE_PIPELINE_LOADER_HPP

#include <memory>
#include <string>
#include "DefinitionTypes.hpp"
#include <unordered_map>
#include <boost/thread/shared_mutex.hpp>

class GlShader;
class Drawbuffer;
class Material;
class ShaderProgram;
class Texture;

class Loader {
public:
  std::shared_ptr<Material> loadMaterial(std::string materia);
  std::shared_ptr<Drawbuffer> loadMesh(std::string model);

  void addMaterial(std::string name, MaterialDef def);
private:
  void buildMaterial(std::shared_ptr<Material> mat, std::pair<std::string, MaterialDef> matdef);
  std::shared_ptr<Texture> loadTexture(std::string path);
  enum ShaderType {
    Vertex,
    Fragment,
    Geometry,
  };
  std::shared_ptr<GlShader> loadShader(std::string path, ShaderType type);
  std::shared_ptr<ShaderProgram> loadProgram(ShaderKey key);

  std::unordered_map<std::string, std::weak_ptr<Material>> materials;
  std::unordered_map<std::string, MaterialDef> material_defs;
  std::unordered_map<std::string, std::weak_ptr<Drawbuffer>> meshes;
  std::unordered_map<ShaderKey, std::weak_ptr<ShaderProgram>> programs;
  std::unordered_map<std::string, std::weak_ptr<GlShader>> shaders;
  std::unordered_map<std::string, std::weak_ptr<Texture>> textures;

  void init();
  void run();
  void cleanup();

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

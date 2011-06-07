#ifndef SENSE_PIPELINE_LOADER_HPP
#define SENSE_PIPELINE_LOADER_HPP

#include <memory>
#include <string>
#include "DefinitionTypes.hpp"

class GlShader;
class Drawbuffer;
class Material;
class Texture;

class Loader {
public:
  std::shared_ptr<Drawbuffer> loadMesh(std::string model);
  std::shared_ptr<Material> loadMaterial(std::string material);

private:
  std::shared_ptr<Texture> loadTexture(std::string path);
  std::shared_ptr<GlShader> loadShader(std::string path);

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

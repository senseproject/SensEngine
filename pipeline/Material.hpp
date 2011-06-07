#ifndef SENSE_PIPELINE_MATERIAL_HPP
#define SENSE_PIPELINE_MATERIAL_HPP

#include "DefinitionTypes.hpp"

#include <memory>
#include <vector>

struct ShaderSet;
struct Texture;

class Uniform {
  UniformDef::Type type;
  std::shared_ptr<Texture> texture;
  unsigned int gl_id;

  friend class Pipeline;
  friend class Material;
};

struct GlShader {
private:
  ~GlShader(); // GlShaders delete themselves when they're destroyed

  unsigned int gl_id;

  enum {
    Vertex,
    Geometry,
    Fragment
  };

  GlShader() : gl_id(0) {};

  friend class Pipeline;
  friend class ShaderProgram;
};

class ShaderProgram {
  ~ShaderProgram(); // ShaderProgram cleans itself up when its destroyed

  unsigned int gl_id;
  std::shared_ptr<GlShader> vert;
  std::shared_ptr<GlShader> geom;
  std::shared_ptr<GlShader> frag;

  ShaderProgram() : gl_id(0) {}; // ShaderProgram is initialized as-needed by the material creation code, this just inits all to 0

  friend class Pipeline;
  friend class Material;

#ifndef _MSC_VER
  ShaderProgram(const ShaderProgram&)=delete;
  ShaderProgram& operator=(const ShaderProgram&)=delete;
#else
  // visual studio 2010 still doesn't support =delete, so just leave them unimplemented
  // for the linker to throw an error on instead of the compiler
  ShaderProgram(const ShaderProgram&);
  ShaderProgram& operator=(const ShaderProgram&);
#endif
};

class Material {
private:
  std::vector<Uniform> uniforms;
  std::shared_ptr<ShaderProgram> prog;

#ifndef _MSC_VER
  Material() =default;
  Material(const Material&)=delete;
  Material& operator=(const Material&)=delete;
#else
  // visual studio 2010 still doesn't support =delete, so just leave them unimplemented
  // for the linker to throw an error on instead of the compiler
  Material() {}
  Material(const Material&);
  Material& operator=(const Material&);
#endif

  friend class Pipeline;
};

#endif // SENSE_PIPELINE_MATERIAL_HPP


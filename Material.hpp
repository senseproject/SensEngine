#ifndef QNT_RENDERER_MATERIAL_H
#define QNT_RENDERER_MATERIAL_H

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

  ShaderProgram(const ShaderProgram&)=delete;
  ShaderProgram& operator=(const ShaderProgram&)=delete;
};

class Material {
private:
  std::vector<Uniform> uniforms;
  std::shared_ptr<ShaderProgram> prog;

  Material() {};
  Material(const Material&);
  Material& operator=(const Material&);

  friend class Pipeline;
};

#endif

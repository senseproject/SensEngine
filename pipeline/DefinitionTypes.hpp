#ifndef QNT_RENDERER_DEFINITIONTYPES_H
#define QNT_RENDERER_DEFINITIONTYPES_H

#include <memory>
#include <string>
#include <unordered_map>

struct ShaderKey {
  std::string vert;
  std::string geom;
  std::string frag;

  static std::shared_ptr<ShaderKey> create() { return std::shared_ptr<ShaderKey>(new ShaderKey); }

private:
  ShaderKey() {};
  ShaderKey(const ShaderKey&);
  ShaderKey& operator=(const ShaderKey&);
};

struct UniformDef {
  enum Type {
    SpecialTexture,
    Texture,
    ModelView,
    Projection,
    DepthInfo,

    // light-specific types
    LightColor,
    LightRadius,
    LightPosition,

    // Other special types
    BoneMatrices,
  } type;
  // I want so badly to make these a union, but constructors are in my way here
  std::string texture;

  static std::shared_ptr<UniformDef> create() { return std::shared_ptr<UniformDef>(new UniformDef); }
};

struct MaterialDef {
  std::shared_ptr<ShaderKey> shaders;
  std::unordered_map<std::string, std::shared_ptr<UniformDef> > uniforms;

  static std::shared_ptr<MaterialDef> create() { return std::shared_ptr<MaterialDef>(new MaterialDef); }

private:
  MaterialDef() { shaders = ShaderKey::create(); }
  MaterialDef(const MaterialDef&);
  MaterialDef& operator=(const MaterialDef&);
};

#endif

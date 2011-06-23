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

#ifndef SENSE_PIPELINE_DEFINITIONTYPES_HPP
#define SENSE_PIPELINE_DEFINITIONTYPES_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/any.hpp>
#include <boost/functional/hash.hpp>

struct ShaderKey {
  std::string vert;
  std::string geom;
  std::string frag;
};

inline bool operator==(const ShaderKey& lhs, const ShaderKey& rhs) {
  return lhs.vert == rhs.vert && lhs.frag == rhs.frag && lhs.geom == rhs.geom;
}

namespace std {
  template <>
  struct hash<ShaderKey> : public unary_function<ShaderKey, size_t> {
    size_t operator()(const ShaderKey& k) const {
      size_t result = 0;
      boost::hash_combine(result, k.vert);
      boost::hash_combine(result, k.frag);
      boost::hash_combine(result, k.geom);
      return result;
    }
  };
}

struct UniformDef {
  enum Type {
    Texture,
    Webview,
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
  boost::any value;
};

struct MaterialDef {
  ShaderKey shaders;
  std::unordered_map<std::string, UniformDef> uniforms;
};

#endif // SENSE_PIPELINE_DEFINITIONTYPES_HPP

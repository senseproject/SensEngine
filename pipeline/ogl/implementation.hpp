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

#ifndef SENSE_PIPELINE_OGL_IMPLEMENTATION_HPP
#define SENSE_PIPELINE_OGL_IMPLEMENTATION_HPP

#include "GL/glew.h"
#include "../DefinitionTypes.hpp"

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class SenseClient;

struct DrawableMesh;
struct Material;

struct GlShader
{
  GLuint refcnt;
  GLuint gl_id;
  enum Type {
    Vertex,
    Geometry,
    Fragment,
  };
};

struct ShaderProgram
{
  GLuint refcnt;
  GLuint gl_id;
  GlShader* vert;
  GlShader* geom;
  GlShader* frag;
};

struct ShaderSet
{
  GlShader* vert;
  GlShader* geom;
  GlShader* frag;
};

inline bool operator==(const ShaderSet& lhs, const ShaderSet& rhs) {
  return lhs.vert == rhs.vert && lhs.frag == rhs.frag && lhs.geom == rhs.geom;
}

namespace std {
  template <>
  struct hash<ShaderSet> : public unary_function<ShaderSet, size_t> {
    size_t operator()(const ShaderSet& k) const {
      size_t result = 0;
      boost::hash_combine(result, k.vert);
      boost::hash_combine(result, k.frag);
      boost::hash_combine(result, k.geom);
      return result;
    }
  };
}

struct Image;

struct Texture
{
  GLuint id;
};

struct RenderTarget
{
  GLuint lbuffer_id;
  GLuint gbuffer_id;
  GLuint depth_id;
  GLuint color_id;
  GLuint normal_id;
  GLuint lighting_id;
  bool build_mips;
  bool dirty;
  uint32_t width, height;
};

struct DrawableBuffer
{
  GLuint vtxbuffer;
  GLuint idxbuffer;
  GLuint vao;
  GLenum idx_type;
};

struct DrawTask
{
  DrawableMesh* mesh;
  Material* mat;
};

struct PipelineImpl 
{
  RenderTarget* current_framebuffer;

  GLuint cur_fsaa;
  std::set<uint32_t> fsaa_levels;

  GLuint width, height;

  std::vector<DrawTask> tasks;
};

struct LoaderImpl
{
  std::string shader_header;
  std::unordered_map<ShaderSet, ShaderProgram*> programs;
  std::unordered_map<std::string, GlShader*> shaders;
  std::unordered_map<std::string, Texture*> textures;
  std::unordered_set<DrawableMesh*> meshes;

  GlShader* loadShader(std::string, GLenum);
};

#endif // SENSE_PIPELINE_OGL_IMPLEMENTATION_HPP

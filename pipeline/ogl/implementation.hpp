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

#ifndef SENSE_PIPELINE_OGL_PIPELINE_P_HPP
#define SENSE_PIPELINE_OGL_PIPELINE_P_HPP

#include "GL/glew.h"
#include "../DefinitionTypes.hpp"

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class SenseClient;

struct Uniform
{
  UniformDef::Type type;
  boost::any value;
  GLint gl_id;
};

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

struct Material
{
  GLuint refcnt;
  std::vector<Uniform> uniforms;
  ShaderProgram* program;
};

struct Texture
{
  virtual ~Texture();
  
  GLuint refcnt;
  GLuint gl_id;
  volatile GLuint biggest_mip_loaded;
  std::string name;
  GLint hres, vres;
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

struct PipelineImpl 
{
  SenseClient* client;
  RenderTarget* current_framebuffer;

  GLuint cur_fsaa;
  std::set<uint32_t> fsaa_levels;
};

struct LoaderImpl
{
  SenseClient* client;
  volatile bool finished;
  std::string shader_header;
  std::unordered_map<std::string, MaterialDef> material_defs;
  std::unordered_map<std::string, Material*> materials;
//  std::unordered_map<std::string, std::weak_ptr<DrawBuffer>> meshes;
  std::unordered_map<ShaderKey, ShaderProgram*> programs;
  std::unordered_map<std::string, GlShader*> shaders;
  std::unordered_map<std::string, Texture*> textures;

  void buildMaterial(Material*, std::pair<std::string, MaterialDef>);
  Texture* loadTexture(std::string);
  Texture* loadWebview(std::string);
  GlShader* loadShader(std::string, GlShader::Type);
  ShaderProgram* loadProgram(ShaderKey);

  void cleanupMaterial(Material*);
};

#endif // SENSE_PIPELINE_OGL_PIPELINE_P_HPP

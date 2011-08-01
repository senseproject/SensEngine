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

#ifndef SENSE_PIPELINE_MATERIAL_HPP
#define SENSE_PIPELINE_MATERIAL_HPP

#include "DefinitionTypes.hpp"

struct ShaderProgram;

struct Uniform
{
  UniformDef::Type type;
  boost::any value;
  boost::any pipe_id;
};

struct Material
{
  ShaderProgram* shaders;
  std::vector<Uniform> uniforms;
  uint32_t refcnt;
};


#endif // SENSE_PIPELINE_MATERIAL_HPP

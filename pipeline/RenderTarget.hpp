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

#ifndef SENSE_PIPELINE_RENDERTARGET_HPP
#define SENSE_PIPELINE_RENDERTARGET_HPP

#include <vector>
#include <memory>

class Texture;

class RenderTarget {
public:
  ~RenderTarget();
private:
  RenderTarget();

  unsigned int gl_fboid;
  std::vector<std::shared_ptr<Texture>> bound_textures;
  bool build_mips;

  friend class Pipeline;

#ifndef _MSC_VER
  RenderTarget(const RenderTarget&)=delete;
  RenderTarget& operator=(const RenderTarget&)=delete;
#else
  // visual studio 2010 still doesn't support =delete, so just leave them unimplemented
  // for the linker to throw an error on instead of the compiler
  RenderTarget(const RenderTarget&);
  RenderTarget& operator=(const RenderTarget&);
#endif
};

#endif // SENSE_PIPELINE_RENDERTARGET_HPP

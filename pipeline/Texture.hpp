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

#ifndef SENSE_PIPELINE_TEXTURE_H
#define SENSE_PIPELINE_TEXTURE_H

#include <memory>
#include <string>

class Texture {
public:
  ~Texture();
private:

  unsigned int gl_texid;
  volatile unsigned int biggest_mip_loaded;
  std::string name;

  Texture();

  friend class Pipeline;

#ifndef _MSC_VER
  Texture(const Texture&) =delete;
  Texture& operator=(const Texture&) =delete;
#else
  // visual studio 2010 still doesn't support =delete, so just leave them unimplemented
  // for the linker to throw an error on instead of the compiler
  Texture(const Texture&);
  Texture& operator=(const Texture&);
#endif
};

#endif // SENSE_PIPELINE_TEXTURE_H

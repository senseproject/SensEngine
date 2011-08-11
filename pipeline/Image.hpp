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

#ifndef SENSE_PIPELINE_IMAGE_HPP
#define SENSE_PIPELINE_IMAGE_HPP

struct Texture;

struct Image
{
  char* data;
  enum Format {
    R8,
    RG8,
    RGB8,
    RGBA8,
  };

  Format format;
  Texture* tex;

  unsigned int width, height;
  bool pipe_build_mips;
};

#endif // SENSE_PIPELINE_IMAGE_HPP

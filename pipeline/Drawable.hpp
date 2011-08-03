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

#ifndef SENSE_PIPELINE_DRAWABLE_HPP
#define SENSE_PIPELINE_DRAWABLE_HPP

struct DrawableBuffer;

struct DrawableMesh {
  enum AttribType {
    Byte,
    UByte,
    Short,
    UShort,
    Int,
    UInt,
    Float,
    Half,
    Double
  };

  enum AttribLocation {
    Pos,
    Nor,
    Tan,
    Col,
    Te0,
    Te1,
    SkinIdx,
    SkinWeight,
    UserStart
  };

  enum AttribSpecial {
    None,
    Normalize,
    Integer,
  };

  struct Attribute 
  {
    AttribType type;
    AttribLocation loc;
    size_t start;
    uint8_t size;
    AttribSpecial special;
  };

  std::vector<Attribute> attributes;
  void* data;
  size_t data_size;
  size_t data_stride;

  void* index_data;
  size_t index_count;
  AttribType index_type;

  DrawableBuffer* buffer;

  size_t refcnt;
};

#endif // SENSE_PIPELINE_DRAWABLE_HPP

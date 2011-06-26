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

#ifndef SENSE_PIPELINE_DRAWBUFFER_HPP
#define SENSE_PIPELINE_DRAWBUFFER_HPP

#include <memory>

class Renderer;

class DrawBuffer {
public:
  virtual ~DrawBuffer();

protected:
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

private:
  // MUST be called before any other operation (construction implicitly binds until the next bind() call on another object)
  void bind();
  virtual void draw(unsigned int count=1);
  void setData(void* data, size_t vert_count, unsigned int stride, bool stream=false);
  void attribute(unsigned int idx, int size, size_t start, AttribType type, bool normalize=true);
  void intAttribute(unsigned int idx, int size, size_t start, AttribType type);
  
  size_t m_num_verts;
  unsigned int m_stride;
  
  // OpenGL-specific variables
  unsigned int vao_id, vbo_id;

protected:
  DrawBuffer(unsigned int vbo_id=0);

#ifndef _MSC_VER
  DrawBuffer(const DrawBuffer&)=delete;
  DrawBuffer& operator=(const DrawBuffer&)=delete;
#else
  // visual studio 2010 still doesn't support =delete, so just leave them unimplemented
  // for the linker to throw an error on instead of the compiler
  DrawBuffer(const DrawBuffer&);
  DrawBuffer& operator=(const DrawBuffer&);
#endif

  friend class Pipeline;
  friend class Loader;
};

class IndexedDrawBuffer: public DrawBuffer {
  virtual ~IndexedDrawBuffer();

  virtual void draw(unsigned int count=1);
  void setIndices(void* data, size_t num_indices, AttribType type, bool stream=false);

  IndexedDrawBuffer(unsigned int ibo_id=0);

  AttribType m_index_type;
  size_t m_num_indices;

  // OpenGL-specific variables
  unsigned int ibo_id;

#ifndef _MSC_VER
  IndexedDrawBuffer(const IndexedDrawBuffer&)=delete;
  IndexedDrawBuffer& operator=(const IndexedDrawBuffer&)=delete;
#else
  // visual studio 2010 still doesn't support =delete, so just leave them unimplemented
  // for the linker to throw an error on instead of the compiler
  IndexedDrawBuffer(const IndexedDrawBuffer&);
  IndexedDrawBuffer& operator=(const IndexedDrawBuffer&);
#endif
};

#endif // SENSE_PIPELINE_DRAWBUFFER_HPP

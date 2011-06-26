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

#include "GL/glew.h"
#include "Drawbuffer.hpp"

#include "glexcept.hpp"

DrawBuffer::DrawBuffer(unsigned int vbo_id) : m_num_verts(0), m_stride(0), vbo_id(vbo_id) {
  GL_CHECK(glGenVertexArrays(1, &vao_id))
  GL_CHECK(glBindVertexArray(vao_id))
  if(!vbo_id)
    GL_CHECK(glGenBuffers(1, &(this->vbo_id)))
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo_id))
  
  // initialize sane defaults for well-known vertex attribs
  GL_CHECK(glVertexAttrib4f(Pos, 0.f, 0.f, 0.f, 1.f))
  GL_CHECK(glVertexAttrib3f(Nor, 0.f, 0.f, 1.f))
  GL_CHECK(glVertexAttrib3f(Tan, 0.f, 1.f, 0.f))
  GL_CHECK(glVertexAttrib4f(Col, 1.f, 1.f, 1.f, 1.f))
  GL_CHECK(glVertexAttrib3f(Te0, 0.f, 0.f, 0.f))
  GL_CHECK(glVertexAttrib3f(Te1, 0.f, 0.f, 0.f))
  GL_CHECK(glVertexAttribI4i(SkinIdx, 0, 0, 0, 0))
  GL_CHECK(glVertexAttrib4f(SkinWeight, 0.f, 0.f, 0.f, 0.f))
}

DrawBuffer::~DrawBuffer() {
  GL_CHECK(glDeleteBuffers(1, &vbo_id))
  GL_CHECK(glDeleteVertexArrays(1, &vao_id))
}

void DrawBuffer::bind() {
  GL_CHECK(glBindVertexArray(vao_id))
}

void DrawBuffer::draw(unsigned int count) {
  GL_CHECK(glDrawArraysInstancedARB(GL_TRIANGLES, 0, m_num_verts, count))
}

void DrawBuffer::setData(void* data, size_t vert_count, unsigned int stride, bool stream) {
  GLenum usage = stream ? GL_STREAM_DRAW : GL_STATIC_DRAW;
  GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vert_count*stride, data, usage))
  m_stride = stride;
  m_num_verts = vert_count;
}

void DrawBuffer::attribute(unsigned int idx, int size, size_t start, DrawBuffer::AttribType type, bool normalize) {
  GLenum gltype;
  switch(type) {
    case Byte:
      gltype = GL_BYTE; break;
    case UByte:
      gltype = GL_UNSIGNED_BYTE; break;
    case Short:
      gltype = GL_SHORT; break;
    case UShort:
      gltype = GL_UNSIGNED_SHORT; break;
    case Int:
      gltype = GL_INT; break;
    case UInt:
      gltype = GL_UNSIGNED_INT; break;
    case Float:
      gltype = GL_FLOAT; break;
    case Half:
      gltype = GL_HALF_FLOAT; break;
    case Double:
      gltype = GL_DOUBLE; break;
  }
  GL_CHECK(glEnableVertexAttribArray(idx))
  GL_CHECK(glVertexAttribPointer(idx, size, gltype, normalize, m_stride, (void*)start))
}

void DrawBuffer::intAttribute(unsigned int idx, int size, size_t start, DrawBuffer::AttribType type) {
    GLenum gltype;
  switch(type) {
    case Byte:
      gltype = GL_BYTE; break;
    case UByte:
      gltype = GL_UNSIGNED_BYTE; break;
    case Short:
      gltype = GL_SHORT; break;
    case UShort:
      gltype = GL_UNSIGNED_SHORT; break;
    case Int:
      gltype = GL_INT; break;
    case UInt:
      gltype = GL_UNSIGNED_INT; break;
    default:
      throw std::logic_error("Can't create an integer attribute from a float value");
  }
  GL_CHECK(glEnableVertexAttribArray(idx))
  GL_CHECK(glVertexAttribIPointer(idx, size, gltype, m_stride, (void*)start))
}

IndexedDrawBuffer::IndexedDrawBuffer(unsigned int ibo_id) : DrawBuffer(), ibo_id(ibo_id) {
  if(!ibo_id)
    GL_CHECK(glGenBuffers(1, &(this->ibo_id)))
  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_id))
}

IndexedDrawBuffer::~IndexedDrawBuffer() {
  glDeleteBuffers(1, &ibo_id);
}

void IndexedDrawBuffer::draw(unsigned int count) {
  GLenum type;
  switch(m_index_type) {
    case UByte: type = GL_UNSIGNED_BYTE; break;
    case UShort: type = GL_UNSIGNED_SHORT; break;
    case UInt: type = GL_UNSIGNED_INT;
    default: throw std::runtime_error("Unhandled index type in draw call");
  }
  GL_CHECK(glDrawElementsInstancedARB(GL_TRIANGLES, m_num_indices, type, 0, count))
}

void IndexedDrawBuffer::setIndices(void* data, size_t num_indices, DrawBuffer::AttribType type, bool stream) {
  int size;
  switch(type) {
    case UByte: size = 1; break;
    case UShort: size = 2; break;
    case UInt: size = 4; break;
    default: throw std::runtime_error("Only UByte, UShort, and UInt are allowed index types");
  }
  GLenum usage;
  if(stream)
    usage = GL_STREAM_DRAW;
  else
    usage = GL_STATIC_DRAW;
  GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*size, data, usage))
  m_index_type = type;
  m_num_indices = num_indices;
}

#include "GL/glew.h"
#include "Drawbuffer.hpp"

#include <stdexcept>

DrawBuffer::DrawBuffer() : m_num_verts(0), m_stride(0) {
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);
  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

  // initialize sane defaults for well-known vertex attribs
  glVertexAttrib4f(Pos, 0.f, 0.f, 0.f, 1.f);
  glVertexAttrib3f(Nor, 0.f, 0.f, 1.f);
  glVertexAttrib3f(Tan, 0.f, 1.f, 0.f);
  glVertexAttrib4f(Col, 1.f, 1.f, 1.f, 1.f);
  glVertexAttrib3f(Te0, 0.f, 0.f, 0.f);
  glVertexAttrib3f(Te1, 0.f, 0.f, 0.f);
  glVertexAttribI4i(SkinIdx, 0, 0, 0, 0);
  glVertexAttrib4f(SkinWeight, 0.f, 0.f, 0.f, 0.f);
}

DrawBuffer::~DrawBuffer() {
  glDeleteBuffers(1, &vbo_id);
  glDeleteVertexArrays(1, &vao_id);
}

void DrawBuffer::bind() {
  glBindVertexArray(vao_id);
}

void DrawBuffer::draw(unsigned int count) {
  glDrawArraysInstancedARB(GL_TRIANGLES, 0, m_num_verts, count);
}

void DrawBuffer::setData(void* data, size_t vert_count, unsigned int stride, bool stream) {
  GLenum usage = stream ? GL_STREAM_DRAW : GL_STATIC_DRAW;
  glBufferData(GL_ARRAY_BUFFER, vert_count*stride, data, usage);
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
  glEnableVertexAttribArray(idx);
  glVertexAttribPointer(idx, size, gltype, normalize, m_stride, (void*)start);
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
  glEnableVertexAttribArray(idx);
  glVertexAttribIPointer(idx, size, gltype, m_stride, (void*)start);
}

IndexedDrawBuffer::IndexedDrawBuffer() : DrawBuffer() {
  glGenBuffers(1, &ibo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
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
  glDrawElementsInstancedARB(GL_TRIANGLES, m_num_indices, type, 0, count);
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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*size, data, usage);
  m_index_type = type;
  m_num_indices = num_indices;
}

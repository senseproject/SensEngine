#ifndef QNT_RENDERER_DRAWBUFFER_H
#define QNT_RENDERER_DRAWBUFFER_H

#include <memory>

class Renderer;

class DrawBuffer {
protected:
  virtual ~DrawBuffer();

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
  DrawBuffer();

  DrawBuffer(const DrawBuffer&)=delete;
  DrawBuffer& operator=(const DrawBuffer&)=delete;

  friend class Pipeline;
};

class IndexedDrawBuffer: public DrawBuffer {
  virtual ~IndexedDrawBuffer();

  virtual void draw(unsigned int count=1);
  void setIndices(void* data, size_t num_indices, AttribType type, bool stream=false);

  IndexedDrawBuffer();

  AttribType m_index_type;
  size_t m_num_indices;

  // OpenGL-specific variables
  unsigned int ibo_id;

  IndexedDrawBuffer(const DrawBuffer&)=delete;
  IndexedDrawBuffer& operator=(const IndexedDrawBuffer&)=delete;
};

#endif

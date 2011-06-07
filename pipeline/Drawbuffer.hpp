#ifndef SENSE_PIPELINE_DRAWBUFFER_HPP
#define SENSE_PIPELINE_DRAWBUFFER_HPP

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
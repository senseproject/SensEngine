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

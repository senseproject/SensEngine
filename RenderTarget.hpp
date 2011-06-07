#ifndef QNT_RENDERER_RENDERTARGET_HPP
#define QNT_RENDERER_RENDERTARGET_HPP

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

  RenderTarget(const RenderTarget&)=delete;
  RenderTarget& operator=(const RenderTarget&)=delete;
};

#endif // QNT_RENDERER_RENDERTARGET_HPP

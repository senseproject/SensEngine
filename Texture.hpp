#ifndef QNT_RENDERER_TEXTURE_H
#define QNT_RENDERER_TEXTURE_H

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

  Texture(const Texture&) =delete;
  Texture& operator=(const Texture&) =delete;
};

#endif // QNT_RENDERER_TEXTURE_H

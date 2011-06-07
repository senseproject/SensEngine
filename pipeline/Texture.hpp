#ifndef SENSE_PIPELINE_TEXTURE_H
#define SENSE_PIPELINE_TEXTURE_H

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

#ifndef _MSC_VER
  Texture(const Texture&) =delete;
  Texture& operator=(const Texture&) =delete;
#else
  // visual studio 2010 still doesn't support =delete, so just leave them unimplemented
  // for the linker to throw an error on instead of the compiler
  Texture(const Texture&);
  Texture& operator=(const Texture&);
#endif
};

#endif // SENSE_PIPELINE_TEXTURE_H

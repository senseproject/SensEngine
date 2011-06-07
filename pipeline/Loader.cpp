#include "Loader.hpp"
#include "Material.hpp"
#include "GL/glew.h"
#include "glexcept.hpp"

void Loader::init() {
}

void Loader::run() {
}

void Loader::cleanup() {
}

std::shared_ptr<Material> Loader::loadMaterial(std::string material) {
  auto it = materials.find(material);
  if(it != materials.end() && !it->second.expired())
    return it->second.lock();

  auto i = material_defs.find(material);
  std::shared_ptr<Material> mat;
  if(i != material_defs.end()) {
    mat = std::shared_ptr<Material>(new Material);
    buildMaterial(mat, *i);
  }
  return mat;
}

std::shared_ptr<Drawbuffer> Loader::loadMesh(std::string model) {
  auto it = meshes.find(model);
  if(it != meshes.end() && !it->second.expired())
    return it->second.lock();
  return std::shared_ptr<Drawbuffer>(); // TODO: load the mesh
}

void Loader::addMaterial(std::string name, MaterialDef def) {
  auto i = material_defs.insert(std::make_pair(name, def));
  auto it = materials.find(name);
  if(it != materials.end() && !it->second.expired())
      buildMaterial(it->second.lock(), *(i.first));
}

void Loader::buildMaterial(std::shared_ptr<Material> mat, std::pair<std::string, MaterialDef> matdef) {
  mat->program = loadProgram(matdef.second.shaders);
  GL_CHECK(glUseProgram(mat->program->gl_id))
  for(auto i = matdef.second.uniforms.begin(); i != matdef.second.uniforms.end(); ++i) {
    Uniform u;
    u.gl_id = glGetUniformLocation(mat->program->gl_id, i->first.c_str());
    if(u.gl_id == -1)
      continue;
    u.type = i->second.type;
    if(u.type == UniformDef::Texture)
      u.value = loadTexture(boost::any_cast<std::string>(i->second.value));
    mat->uniforms.push_back(u);
  }
}

std::shared_ptr<Texture> Loader::loadTexture(std::string path) {
  auto it = textures.find(path);
  if(it != textures.end() && !it->second.expired())
    return it->second.lock();
  return std::shared_ptr<Texture>(); // TODO: create a texture object and add a job to the loader thread
}

std::shared_ptr<GlShader> Loader::loadShader(std::string path, ShaderType type) {
  auto it = shaders.find(path);
  if(it != shaders.end() && !it->second.expired())
    return it->second.lock();

  GLenum gl_shader_type;
  switch(type) {
    case Vertex: gl_shader_type = GL_VERTEX_SHADER; break;
    case Fragment: gl_shader_type = GL_FRAGMENT_SHADER; break;
    case Geometry: gl_shader_type = GL_GEOMETRY_SHADER; break;
  }
  
  return std::shared_ptr<GlShader>(); // TODO: create a shader object and compile the shader
}

std::shared_ptr<ShaderProgram> Loader::loadProgram(ShaderKey key) {
  auto it = programs.find(key);
  if(it != programs.end() && !it->second.expired())
    return it->second.lock();
  std::shared_ptr<ShaderProgram> prog = std::shared_ptr<ShaderProgram>(new ShaderProgram);
  prog->gl_id = GL_CHECK(glCreateProgram())
  if(!key.frag.empty())
    prog->frag = loadShader(key.frag, Fragment);
  else
    throw std::runtime_error("Programs must have at least a vertex and fragment shader (missing fragment)");
  if(!key.vert.empty())
    prog->vert = loadShader(key.vert, Vertex);
  else
    throw std::runtime_error("Programs must have at least a vertex and fragment shader (missing vertex)");
  if(!key.geom.empty())
    prog->geom = loadShader(key.geom, Geometry);
  GL_CHECK(glAttachShader(prog->gl_id, prog->vert->gl_id))
  GL_CHECK(glAttachShader(prog->gl_id, prog->geom->gl_id))
  GL_CHECK(glAttachShader(prog->gl_id, prog->frag->gl_id))
  GL_CHECK(glLinkProgram(prog->gl_id))
  int link_status;
  GL_CHECK(glGetProgramiv(prog->gl_id, GL_LINK_STATUS, &link_status))
  if(link_status == GL_FALSE) {
    int info_log_length;
    GL_CHECK(glGetProgramiv(prog->gl_id, GL_INFO_LOG_LENGTH, &info_log_length))
    char *log = new char[info_log_length];
    GL_CHECK(glGetProgramInfoLog(prog->gl_id, info_log_length, &info_log_length, log))
    std::string infolog = log;
    delete[] log;
    throw std::runtime_error("Error linking program: \n" + infolog);
  }
  programs.insert(std::make_pair(key, prog));
  return prog;
}

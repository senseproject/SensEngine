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

#include "DataManager.hpp"
#include "Builtins.hpp"
#include "pipeline/Drawable.hpp"
#include "pipeline/Image.hpp"
#include "pipeline/Material.hpp"
#include "pipeline/interface.hpp"

#include <boost/thread/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <png.h>

enum {
  BUILD_MATERIAL,
  LOAD_TEXTURE,
  LOAD_MESH,

  FINISH_MESH_LOAD,
};

DataManager::DataManager(Loader* loader)
  : m_loader(loader)
{
  loadBuiltinData();
}

DataManager::~DataManager()
{}

void DataManager::exec()
{
  using boost::any_cast;

  m_finished = false;
  job j;
  while(!m_finished) {
    while(m_jobs.try_pop(j)) {
      switch(j.first) {
      case BUILD_MATERIAL:
        buildMaterial(any_cast<std::string>(j.second));
        break;
      case LOAD_TEXTURE:
        loadTexture(any_cast<std::string>(j.second));
        break;
      case LOAD_MESH:
	loadMeshFile(any_cast<std::string>(j.second));
      }
    }
    boost::this_thread::yield();
  }
}

void DataManager::finish()
{
  m_finished = true;
}

void DataManager::mainThreadTick()
{
  using boost::any_cast;

  job j;
  while(m_main_thread_jobs.try_pop(j)) {
    switch(j.first) {
    case FINISH_MESH_LOAD: 
      m_loader->mainThreadLoadMesh(any_cast<DrawableMesh*>(j.second));
      break;
    }
  }
}

Material* DataManager::loadMaterial(std::string name)
{
  auto i = m_materials.find(name);
  if(i != m_materials.end()) {
    i->second->refcnt++;
    return i->second;
  } else {
    Material* m = new Material;
    m->shaders = 0;
    m->refcnt = 0;
    m_materials.insert(std::make_pair(name, m));
    m_jobs.push(job(BUILD_MATERIAL, name));
    return m;
  }
}

void DataManager::addMaterial(MaterialDef def, std::string name)
{
  bool inserted = m_matdefs.insert(std::make_pair(name, def)).second;
  if(!inserted) {
    m_matdefs[name] = def;
    if(m_materials.find(name) != m_materials.end()) {
      // there are instances of this material. Reload the sucker!
      m_jobs.push(job(BUILD_MATERIAL, name));
    }
  }
}

DrawableMesh* DataManager::loadMesh(std::string name)
{
  auto i = m_meshes.find(name);
  if(i != m_meshes.end()) {
    i->second->refcnt++;
    return i->second;
  }
  DrawableMesh* msh = new DrawableMesh;
  msh->buffer = 0;
  m_meshes.insert(std::make_pair(name, msh));
  m_jobs.push(job(LOAD_MESH, name));
  return msh;
}

void DataManager::buildMaterial(std::string name)
{
  MaterialDef def = m_matdefs[name];
  std::string vert, frag, geom;
  vert = loadShaderString(def.shaders.vert + ".vs");
  frag = loadShaderString(def.shaders.frag + ".fs");
  geom = loadShaderString(def.shaders.geom + ".gs");

  ShaderProgram* s = m_loader->loadProgram(vert, frag, geom);
  std::vector<Uniform> uniforms;
  for(auto i = def.uniforms.begin(); i!= def.uniforms.end(); ++i) {
    Uniform u;
    u.type = i->second.type;
    if(u.type == UniformDef::Texture) {
      std::string name = boost::any_cast<std::string>(i->second.value);
      auto j = m_images.find(name);
      if(j == m_images.end()) {
        Image* img = new Image;
        img->data = 0;
        img->tex = 0;
        m_imglock.lock();
        m_images.insert(std::make_pair(name, img));
        m_imglock.unlock();
        u.value = img;
        m_jobs.push(job(LOAD_TEXTURE, name));
      } else {
        u.value = i->second;
      }
    } else {
      u.value = i->second.value;
    }
    u.pipe_id = m_loader->queryUniform(s, i->first);
    uniforms.push_back(u);
  }

  Material *m;
  auto i = m_materials.find(name);
  if(i != m_materials.end()) {
    m = i->second;
    m_loader->releaseProgram(m->shaders);
  } else {
    // This *should* never be a valid codepath. But just in case...
    m = new Material;
    m->refcnt = 0;
    m_materials.insert(std::make_pair(name, m));
  }
  m->uniforms = uniforms;
  m->shaders = s;
}

std::string DataManager::loadShaderString(std::string name)
{
  if(name.size() == 3 && name[0] == '.' && name[2] == 's' &&
     ( name[1] == 'f' || name[1] == 'v' || name[1] == 'g'))
    return ""; // empty shader name means empty shader string
  auto i = m_shaderstrings.find(name);
  if(i != m_shaderstrings.end())
    return i->second;

  boost::filesystem::path shader_path("../data/shaders");
  shader_path = shader_path / name;
  if(!exists(shader_path))
    throw std::runtime_error("Can't find shader file " + shader_path.string());
  boost::filesystem::ifstream stream;
  stream.open(shader_path);
  std::string shader =  std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
  m_shaderstrings.insert(std::make_pair(name, shader));
  return shader;
}

namespace {
  void readPngData(png_structp pngPtr, png_bytep data, png_size_t length) {
    png_voidp a = png_get_io_ptr(pngPtr);
    ((std::istream*)a)->read((char*)data, length);
  }
}

void DataManager::loadTexture(std::string name)
{
  boost::filesystem::path img_path("../data/textures");
  img_path = img_path / (name+".png");
  if(!exists(img_path))
    throw std::runtime_error("Can't find texture file " + img_path.string());
  boost::filesystem::ifstream stream;
  stream.open(img_path, std::ios_base::binary);

  char pngsig[8];
  stream.read(pngsig, 8);
  int is_png = png_sig_cmp((png_bytep)pngsig, 0, 8);
  if(is_png != 0)
    throw std::runtime_error(img_path.string() + " is not a PNG file");
  png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!pngPtr)
    throw std::runtime_error("Error initializing PNG reader");
  png_infop infoPtr = png_create_info_struct(pngPtr);
  if(!infoPtr) {
    png_destroy_read_struct(&pngPtr, 0, 0);
    throw std::runtime_error("Error initializing PNG reader");
  }
  stream.seekg(0);
  png_set_read_fn(pngPtr, (png_voidp)(&stream), readPngData);
  png_set_sig_bytes(pngPtr, 0);
  png_read_info(pngPtr, infoPtr);

  m_imglock.lock();
  auto iter = m_images.find(name);
  if(iter == m_images.end())
    throw std::runtime_error("Tried to load image that hasn't been created: " + name);
  Image* img = iter->second;
  m_imglock.unlock();
  img->width = png_get_image_width(pngPtr, infoPtr);
  img->height = png_get_image_height(pngPtr, infoPtr);

  png_uint_32 bitdepth = png_get_bit_depth(pngPtr, infoPtr);
  png_uint_32 channels = png_get_channels(pngPtr, infoPtr);
  png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);

  png_set_expand(pngPtr);

  switch(color_type) {
  case PNG_COLOR_TYPE_PALETTE:
    channels = 3;
    break;
  case PNG_COLOR_TYPE_GRAY:
    bitdepth = 8;
    break;
  }

  if(png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) {
    channels++;
  }

  if(bitdepth == 16) {
    png_set_strip_16(pngPtr);
    bitdepth = 8;
  }

  if(bitdepth != 8) {
    png_destroy_read_struct(&pngPtr, &infoPtr,(png_infopp)0);
    throw std::runtime_error("PNGs with bitdepths other than 8 are not supported");
  }

  png_bytep* rowPtrs = new png_bytep[img->height];
  img->data = new char[img->width*img->height*channels];
  img->pipe_build_mips = true;
  const unsigned int stride = img->width * channels;
  for(size_t i = 0; i < img->height; i++) {
    png_uint_32 q = (img->height - i - 1) * stride;
    rowPtrs[i] = (png_bytep)img->data + q;
  }
  png_read_image(pngPtr, rowPtrs);
  delete[] rowPtrs;
  png_destroy_read_struct(&pngPtr, &infoPtr,(png_infopp)0);

  switch(channels) {
  case 1: img->format = Image::R8; break;
  case 2: img->format = Image::RG8; break;
  case 3: img->format = Image::RGB8; break;
  case 4: img->format = Image::RGBA8; break;
  }

  m_loader->loadTexture(img);
}

#pragma pack(push, 1)
namespace {
  static const char sbm_magic[] = "SBM\0";
  const unsigned sbm_hasIndices = 0x01;
  const unsigned sbm_streamData = 0x02;
  const unsigned sbm_calcNormal = 0x06;
  const unsigned sbm_calcTangent = 0x0A;
  const unsigned sbm_calcNorTan = 0x0C; // intentionally not masking Stream here

  const unsigned sbm_attr_normalized = 0x80;
  const unsigned sbm_attr_integer = 0x40;
  const unsigned sbm_sizemask = 0x07; // TODO: save ourselves a bit here

  struct SbmHeader {
    char magic[4];
    uint32_t num_verts;
    uint16_t num_attribs;
    uint16_t vert_stride;
    uint16_t flags;
  };

  struct SbmAttrib {
    uint16_t type;
    uint16_t id;
    uint16_t start_offset;
    uint8_t size; // 1-4. Normalized/Integer are encoded in this field
  };
}
#pragma pack(pop)

void DataManager::loadMeshFile(std::string name)
{
  DrawableMesh* msh = m_meshes[name];

  boost::filesystem::path mdl_path("../data/models");
  mdl_path = mdl_path / (name+".sbm");
  if(!exists(mdl_path))
    throw std::runtime_error("Can't find model file " + mdl_path.string());
  boost::filesystem::ifstream stream;
  stream.open(mdl_path, std::ios_base::binary);

  SbmHeader head;
  stream.read((char*)&head, sizeof(SbmHeader));
  if(memcmp(head.magic, sbm_magic, 4) != 0)
    throw std::runtime_error("QBM signature verification failed");
  if(head.flags & sbm_calcNorTan)
    throw std::runtime_error("Runtime normal/tangent calculation is not yet implemented");
  msh->data_size = head.num_verts*head.vert_stride;
  msh->data_stride = head.vert_stride;
  msh->data = new char[msh->data_size];
  stream.read((char*)msh->data, msh->data_size);

  if(head.flags & sbm_hasIndices) {
    uint16_t idx_count;
    uint16_t idx_type;
    stream.read((char*)&idx_count, 2);
    stream.read((char*)&idx_type, 2);
    msh->index_type = (DrawableMesh::AttribType)idx_type;
    msh->index_count = idx_count;
    switch(msh->index_type) {
    case DrawableMesh::UByte:
      msh->index_data = new char[idx_count];
      stream.read((char*)msh->index_data, idx_count);
      break;
    case DrawableMesh::UShort:
      msh->index_data = new char[idx_count*2];
      stream.read((char*)msh->index_data, idx_count*2);
      break;
    default:
      throw std::runtime_error("Can't read SBM indices: bad index type");
    }
  } else {
    msh->index_data = 0;
    msh->index_count = 0;
  }

  for(size_t i = 0; i < head.num_attribs; ++i) {
    SbmAttrib attr;
    stream.read((char*)&attr, sizeof(SbmAttrib));
    DrawableMesh::Attribute a;
    a.type = (DrawableMesh::AttribType)attr.type;
    a.loc = (DrawableMesh::AttribLocation)attr.id;
    a.start = attr.start_offset;
    a.size = attr.size & sbm_sizemask;
    if(attr.size & sbm_attr_normalized)
      a.special = DrawableMesh::Normalize;
    else if(attr.size & sbm_attr_integer)
      a.special = DrawableMesh::Integer;
    msh->attributes.push_back(a);
  }

  m_loader->loadMesh(msh);
  m_main_thread_jobs.push(job(FINISH_MESH_LOAD, msh));
}

void DataManager::loadBuiltinData()
{
  DrawableMesh* builtin;
  DrawableMesh::Attribute a;

  a.type = DrawableMesh::Float;
  a.special = DrawableMesh::None;

  builtin = new DrawableMesh;
  builtin->refcnt = 1; // builtin data is *never* erased
  builtin->data = builtin_quad_data;
  builtin->data_size = 120;
  builtin->data_stride = 20;
  builtin->index_data = 0;

  a.loc = DrawableMesh::Pos;
  a.start = 0;
  a.size = 3;
  builtin->attributes.push_back(a);

  a.loc = DrawableMesh::Te0;
  a.start = 12;
  a.size = 2;
  builtin->attributes.push_back(a);

  m_meshes.insert(std::make_pair("__quad__", builtin));
  m_loader->loadMesh(builtin);
  m_main_thread_jobs.push(job(FINISH_MESH_LOAD, builtin));

  builtin = new DrawableMesh;
  builtin->refcnt = 1; // builtin data is *never* erased
  builtin->data = builtin_missing_data;
  builtin->data_size = 480;
  builtin->data_stride = 20;
  builtin->index_data = builtin_missing_indices;
  builtin->index_count = 36;
  builtin->index_type = DrawableMesh::UShort;

  a.loc = DrawableMesh::Pos;
  a.start = 0;
  a.size = 3;
  builtin->attributes.push_back(a);

  a.loc = DrawableMesh::Te0;
  a.start = 12;
  a.size = 2;
  builtin->attributes.push_back(a);

  m_meshes.insert(std::make_pair("__missing__", builtin));
  m_loader->loadMesh(builtin);
  m_main_thread_jobs.push(job(FINISH_MESH_LOAD, builtin));
}

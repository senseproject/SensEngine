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

#include <GL/glew.h>

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <cstdio>

#include "Pipeline.hpp"
#include "Drawbuffer.hpp"
#include "Material.hpp"
#include "Panel.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "glexcept.hpp"

#include "3rdparty/glm/gtc/type_ptr.hpp"

void Pipeline::runLoaderThread() {
  try {
  platformInitLoader();
  } catch (std::exception& e) {
    loader_error_string = e.what();
  }
  loader_init_complete = true;

  loader().init();

  while(!loader_be_done) {
    loader().run();
  }

  loader().cleanup();

  platformFinishLoader();
}

Pipeline::hTexture Pipeline::createTargetTexture() {
  hTexture tex(new Texture);
  return tex;
}

Pipeline::hRenderTarget Pipeline::createRenderTarget() {
  hRenderTarget target(new RenderTarget);
  target->build_mips = true; // default to building mipmaps
  return target;
}

Pipeline::Pipeline() : loader_init_complete(false), loader_be_done(false), shadowmap_resolution(512), num_csm_splits(3), width(800), height(600) {
  platformInit();
  glewExperimental = GL_TRUE;
  glewInit();

  platformDetachContext(); // On X11, the context can't be current when we setup the loader thread
  loader_thread = boost::thread(std::mem_fun(&Pipeline::runLoaderThread), this);
  while(!loader_init_complete);
  platformAttachContext();
  if(!loader_error_string.empty()) {
    loader_thread.join();
    platformFinish();
    throw std::runtime_error(loader_error_string.c_str());
  }

  int dsamples, csamples;
  glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &dsamples);
  glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &csamples);
  int fsaa = std::min(dsamples, csamples);
  cur_fsaa = fsaa;
  do {
    fsaa_levels.insert(fsaa);
    fsaa /= 2;
  } while(fsaa > 1);
  

  // Enable some standard state
  GL_CHECK(glEnable(GL_MULTISAMPLE))
  GL_CHECK(glEnable(GL_FRAMEBUFFER_SRGB))
  GL_CHECK(glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE))
  glClearColor(0.8f, 0.8f, 0.9f, 1.f);

  // Create our FBOs
  default_framebuffer = createRenderTarget();
  default_framebuffer->build_mips = false;
  gbuffer_framebuffer = createRenderTarget();
  gbuffer_framebuffer->build_mips = false;
  setupRenderTargets();
  setRenderTarget(default_framebuffer);

  csm_tex_array = createTargetTexture();
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, csm_tex_array->gl_texid))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE))
  GL_CHECK(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, shadowmap_resolution, shadowmap_resolution, num_csm_splits+1, 0, GL_RED, GL_FLOAT, 0))

  shadowmap = createTargetTexture();
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, shadowmap->gl_texid))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE))
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, shadowmap_resolution, shadowmap_resolution, 0, GL_RED, GL_FLOAT, 0))
}

Pipeline::~Pipeline() {
  gbuffer_framebuffer = hRenderTarget();
  default_framebuffer = hRenderTarget();
  current_framebuffer = hRenderTarget();
  csm_tex_array = hTexture();
  shadowmap = hTexture();
  loader_be_done = true;
  loader_thread.join();
  platformFinish();
}

void Pipeline::setupRenderTargets() {
  if(!default_framebuffer || !gbuffer_framebuffer)
    return;
  default_framebuffer->bound_textures.clear();
  gbuffer_framebuffer->bound_textures.clear();

  hTexture main_plane = createTargetTexture();
  hTexture depth = createTargetTexture();
  default_framebuffer->bound_textures.push_back(depth);
  default_framebuffer->bound_textures.push_back(main_plane);
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer->gl_fboid))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, main_plane->gl_texid))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cur_fsaa, GL_RGBA16F, width, height, GL_FALSE))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth->gl_texid))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cur_fsaa, GL_DEPTH24_STENCIL8, width, height, GL_FALSE))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, main_plane->gl_texid, 0))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth->gl_texid, 0))
  GL_CHECK(glViewport(0, 0, width, height))
  FBO_CHECK

  hTexture rgbm = createTargetTexture();
  hTexture nor = createTargetTexture();
  gbuffer_framebuffer->bound_textures.push_back(depth);
  gbuffer_framebuffer->bound_textures.push_back(rgbm);
  gbuffer_framebuffer->bound_textures.push_back(nor);
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_framebuffer->gl_fboid))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rgbm->gl_texid))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cur_fsaa, GL_RGBA8, width, height, GL_FALSE))
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, nor->gl_texid))
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cur_fsaa, GL_RG16F, width, height, GL_FALSE))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth->gl_texid, 0))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, rgbm->gl_texid, 0))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, nor->gl_texid, 0))
  GL_CHECK(glViewport(0, 0, width, height))
  FBO_CHECK
}

void Pipeline::pushKbdCallback(Pipeline::KbdCallback* )
{}

void Pipeline::beginFrame() {
  // TODO: anything that needs to happen at the beginning of a frame
}

void Pipeline::render() {
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_framebuffer->gl_fboid))
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
  // TODO: loop through objects and fill the gbuffer
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, current_framebuffer->gl_fboid)) // our current render target
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT))
  // TODO: loop through lights and fill the framebuffer
  if(current_framebuffer->build_mips) {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0)) // unbind the framebuffer so we can read it TODO: is this needed?
    for(auto i = current_framebuffer->bound_textures.begin(); i != current_framebuffer->bound_textures.end(); ++i) {
      GL_CHECK(glBindTexture(GL_TEXTURE_2D, (*i)->gl_texid))
      GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D))
    }
  }
}

void Pipeline::endFrame() {
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer->gl_fboid)) // TODO: make sure we never need this here
  // first things first: we draw all the panels
  pushProjMat(glm::mat4(1.f));
  for(auto i = panels.begin(); i != panels.end(); ++i) {
    hPanel p = *i;
    p->buf->bind();
    int mv_id = useMaterial(p->mat);
    glUniformMatrix4fv(mv_id, 1, 0, glm::value_ptr(p->matrix));
    p->buf->draw();
  }
  popProjMat();
  GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0))
  // TODO: convert this blit into a tonemapping operation of some sort
  GL_CHECK(glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST))
  platformSwap();
}

void Pipeline::setRenderTarget(hRenderTarget target) {
  if(!target) target = default_framebuffer;
  if(target == current_framebuffer) return;
  current_framebuffer = target;
}

Pipeline::hRenderTarget Pipeline::buildRenderTarget(int w, int h, bool mip) {
  hRenderTarget rt = createRenderTarget();
  hTexture tex = createTargetTexture();
  rt->bound_textures.push_back(tex);
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex->gl_texid))
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, 0))
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
  if(mip) {
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR))
  } else {
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
  }
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, rt->gl_fboid))
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->gl_texid, 0))
  FBO_CHECK
  return rt;
}

Pipeline::GpuMemAvailable Pipeline::queryAvailableMem() {
  GpuMemAvailable meminfo;
  if(GLEW_NVX_gpu_memory_info) {
    int mem;
    GL_CHECK(glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &mem))
    meminfo.buffer = mem;
    meminfo.render = mem;
    meminfo.texture = mem;
  } else if(GLEW_ATI_meminfo) {
    int data[4];
    GL_CHECK(glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, data))
    meminfo.buffer = data[0];
    GL_CHECK(glGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, data))
    meminfo.render = data[0];
    GL_CHECK(glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, data))
    meminfo.texture = data[0];
  } else {
    meminfo.buffer = -1;
    meminfo.render = -1;
    meminfo.texture = -1;
  }
  return meminfo;
}

Pipeline::hPanel Pipeline::createPanel(float xscale, float yscale, std::string material)  {
  Panel *p = new Panel;
  p->mat = loader().loadMaterial(material);
  p->buf = loader().loadMesh("__quad__");
  p->matrix = glm::mat4(xscale, 0.f, 0.f, 0.f,
                        0.f, yscale, 0.f, 0.f,
                        0.f, 0.f, 1.f, 0.f,
                        0.f, 0.f, 0.f, 1.f);
  hPanel panel(p);
  panels.insert(panel);
  return panel;
}

void Pipeline::destroyPanel(Pipeline::hPanel p) {
  panels.erase(p);
}

int Pipeline::useMaterial(Pipeline::hMaterial mat) {
  GL_CHECK(glUseProgram(mat->program->gl_id))
  int mv_id;
  for(auto i = mat->uniforms.begin(); i != mat->uniforms.end(); ++i) {
    if(i->gl_id == -1)
      continue;
    switch(i->type) {
    case UniformDef::ModelView:
      mv_id = i->gl_id;
      break;
    case UniformDef::Projection:
      GL_CHECK(glUniformMatrix4fv(i->gl_id, 1, 0, glm::value_ptr(projection_stack[0])))
      break;
    default:
      throw std::runtime_error("Tried to use an unimplemented uniform type!");
    }
  }
  return mv_id;
}

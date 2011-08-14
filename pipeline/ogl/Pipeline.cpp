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

#include "implementation.hpp"
#include "../interface.hpp"
#include "glexcept.hpp"
#include "../Material.hpp"
#include "../Drawable.hpp"
#include "../Image.hpp"

#include <boost/foreach.hpp>

Pipeline::Pipeline()
  : self(new PipelineImpl)
{
  // Get information on FSAA availability
  int dsamples, csamples;
  GL_CHECK(glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &dsamples));
  GL_CHECK(glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &csamples));
  int fsaa = std::min(dsamples, csamples);
  self->cur_fsaa = fsaa;
  do {
    self->fsaa_levels.insert(fsaa);
    fsaa -= 2;
  } while(fsaa > 1);

  // Enable some standard state
  GL_CHECK(glEnable(GL_MULTISAMPLE));
  GL_CHECK(glEnable(GL_FRAMEBUFFER_SRGB));
  GL_CHECK(glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE));
  GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL_CHECK(glClearColor(0.8f, 0.8f, 0.9f, 1.0f));
}

Pipeline::~Pipeline()
{}

void Pipeline::addDrawTask(DrawableMesh* mesh, Material* mat, glm::mat4 mv, bool instance)
{
  DrawTask d;
  d.mesh = mesh;
  d.mat = mat;
  self->tasks.push_back(d);
}


void Pipeline::render()
{
  if(!self->current_framebuffer)
    return; // Skip rendering if there is no framebuffer
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, self->current_framebuffer->gbuffer_id));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  // TODO: loop through objects and fill the gbuffer
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, self->current_framebuffer->lbuffer_id));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
  // TODO: loop through lights and fill the lbuffer
  self->current_framebuffer->dirty = true;
}
  
void Pipeline::endFrame()
{
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, self->current_framebuffer->lbuffer_id));
  // TODO: draw panels
  BOOST_FOREACH(DrawTask d, self->tasks) {
    if(!d.mesh->buffer || !d.mesh->buffer->vao)
      continue;
    if(!d.mat->shaders || !d.mat->shaders->gl_id)
      continue;

    GL_CHECK(glBindVertexArray(d.mesh->buffer->vao));
    GL_CHECK(glUseProgram(d.mat->shaders->gl_id));

    GLuint texid = 1;
    BOOST_FOREACH(Uniform u, d.mat->uniforms) {
      GLuint uniform_id = boost::any_cast<int>(u.pipe_id);
      if(u.type != UniformDef::Texture)
        continue;
      Image* img = boost::any_cast<Image*>(u.value);
      if(!img->tex)
        continue;

      GL_CHECK(glActiveTexture(GL_TEXTURE0+texid));
      GL_CHECK(glBindTexture(GL_TEXTURE_2D, img->tex->id));
      GL_CHECK(glUniform1i(uniform_id, texid));
      texid++;
    }

    if(!d.mesh->index_data) {
      GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, d.mesh->data_size / d.mesh->data_stride));
    } else {
      GL_CHECK(glDrawElements(GL_TRIANGLES, d.mesh->index_count, d.mesh->buffer->idx_type, 0));
    }
  }
  self->tasks.clear();
  GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
  GL_CHECK(glBlitFramebuffer(0, 0, self->current_framebuffer->width, self->current_framebuffer->height, 0, 0, self->width, self->height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
}

// Here's our render target documentation!
// R,G,B,A = color
// X,Y,Z = Normal components
// S = Specular Blend
// Em = emission
// Se = specular exponent
// 0: [R][G][B][A]
// 1: [X [Y][Z][S]
// 2: [Em  ][Se  ]
//
// This is also (shockingly enough) the packing that the default material shader will expect for texture files
// Alpha is stored even though we'll never use it directly, because it's needed for alpha-to-coverage support
RenderTarget* Pipeline::createRenderTarget(uint32_t width, uint32_t height, bool mipmap)
{
  GLuint texids[5];
  GLuint fboids[2];
  glGenFramebuffers(2, fboids);
  glGenTextures(5, texids);
  RenderTarget rt;
  rt.depth_id = texids[0];
  rt.color_id = texids[1];
  rt.normal_id = texids[2];
  rt.matprop_id = texids[3];
  rt.lighting_id = texids[4];
  rt.lbuffer_id = fboids[0];
  rt.gbuffer_id = fboids[1];

  rt.build_mips = mipmap;
  rt.dirty = true;

  rt.width = width;
  rt.height = height;

  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, rt.lbuffer_id));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rt.lighting_id));
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, self->cur_fsaa, GL_RGBA16F, width, height, GL_FALSE));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rt.depth_id));
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, self->cur_fsaa, GL_DEPTH24_STENCIL8, width, height, GL_FALSE));
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, rt.lighting_id, 0));
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, rt.depth_id, 0));
  GL_CHECK(glViewport(0, 0, width, height));
  FBO_CHECK;
  
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, rt.gbuffer_id));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rt.color_id));
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, self->cur_fsaa, GL_RGBA8, width, height, GL_FALSE));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rt.normal_id));
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, self->cur_fsaa, GL_RGBA8, width, height, GL_FALSE));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rt.matprop_id));
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, self->cur_fsaa, GL_RG16F, width, height, GL_FALSE));
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, rt.color_id, 0));
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, rt.normal_id, 0));
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, rt.matprop_id, 0));
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, rt.depth_id, 0));
  GL_CHECK(glViewport(0, 0, width, height));
  FBO_CHECK;

  RenderTarget* result = new RenderTarget;
  *result = rt;
  return result;
}

void Pipeline::destroyRenderTarget(RenderTarget* rt)
{
  GLuint texids[4];
  GLuint fboids[2];
  texids[0] = rt->depth_id;
  texids[1] = rt->color_id;
  texids[2] = rt->normal_id;
  texids[3] = rt->lighting_id;
  fboids[0] = rt->lbuffer_id;
  fboids[1] = rt->gbuffer_id;
  GL_CHECK(glDeleteTextures(4, texids));
  GL_CHECK(glDeleteFramebuffers(2, fboids));
  delete rt;
}

void Pipeline::setRenderTarget(RenderTarget* rt)
{
  self->current_framebuffer = rt;
}

void Pipeline::setViewport(uint32_t width, uint32_t height)
{
  self->width = width;
  self->height = height;
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  GL_CHECK(glViewport(0, 0, width, height));
}

Loader* Pipeline::createLoader()
{
  return new Loader;
}

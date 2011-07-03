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
#include "client/Client.hpp"
#include "glexcept.hpp"

Pipeline::Pipeline(SenseClient* client)
  : self(new PipelineImpl)
{
  self->client = client;

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
  GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
  GL_CHECK(glBlitFramebuffer(0, 0, self->current_framebuffer->width, self->current_framebuffer->height, 0, 0, self->client->width(), self->client->height(), GL_COLOR_BUFFER_BIT, GL_NEAREST));
}

RenderTarget* Pipeline::createRenderTarget(uint32_t width, uint32_t height, bool mipmap)
{
  GLuint texids[4];
  GLuint fboids[2];
  glGenFramebuffers(2, fboids);
  glGenTextures(4, texids);
  RenderTarget rt;
  rt.depth_id = texids[0];
  rt.color_id = texids[1];
  rt.normal_id = texids[2];
  rt.lighting_id = texids[3];
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
  GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, self->cur_fsaa, GL_RG16F, width, height, GL_FALSE));
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, rt.color_id, 0));
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, rt.normal_id, 0));
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
}

void Pipeline::setRenderTarget(RenderTarget* rt)
{
  self->current_framebuffer = rt;
}


bool Pipeline::isLoaderThreaded()
{
  return true;
}

Loader* Pipeline::createLoader()
{
  return new Loader(self->client);
}

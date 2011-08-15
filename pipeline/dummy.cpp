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

#include "interface.hpp"

Loader::Loader()
{}

Loader::~Loader()
{}

void Loader::loadMesh(DrawableMesh*)
{}

void Loader::mainThreadLoadMesh(DrawableMesh*)
{}

void Loader::releaseMesh(DrawableMesh*)
{}

boost::any Loader::queryUniform(ShaderProgram*, std::string)
{
  return -1;
}

ShaderProgram* Loader::loadProgram(std::string, std::string, std::string)
{
  return 0;
}

void Loader::loadTexture(Image*)
{}

void Loader::updateTexture(Image*)
{}

void Loader::releaseTexture(Image*)
{}

bool Loader::isThreaded()
{
  return false;
}

void Loader::releaseProgram(ShaderProgram*)
{}

Pipeline::Pipeline()
{}

Pipeline::~Pipeline()
{}

void Pipeline::addDrawTask(DrawableMesh*, Material*, glm::mat4, Pipeline::RenderPass)
{}

void Pipeline::addLamp(Lamp*)
{}

void Pipeline::render()
{}

void Pipeline::endFrame()
{}

RenderTarget* Pipeline::createRenderTarget(uint32_t, uint32_t, bool)
{ return 0; }

void Pipeline::destroyRenderTarget(RenderTarget*)
{}

void Pipeline::setRenderTarget(RenderTarget*)
{}

void Pipeline::setViewport(uint32_t, uint32_t)
{}

Loader* Pipeline::createLoader()
{ return 0; }

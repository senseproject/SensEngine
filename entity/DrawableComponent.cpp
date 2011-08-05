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

#include "DrawableComponent.hpp"
#include "Entity.hpp"

#include "world/DataManager.hpp"
#include "pipeline/interface.hpp"

DrawableComponent::DrawableComponent(Entity* owner)
  : Component(owner)
{
  m_owner->m_draw = this;

  m_mesh = m_owner->m_datamgr->loadMesh("__quad__");
  m_mat = m_owner->m_datamgr->loadMaterial("simple");
}

DrawableComponent::~DrawableComponent()
{
  m_owner->m_draw = 0;
}

void DrawableComponent::receiveMessage(const Message& msg)
{}

void DrawableComponent::draw(Pipeline* pipe)
{
  pipe->addDrawTask(m_mesh, m_mat, glm::mat4(1.f));
}

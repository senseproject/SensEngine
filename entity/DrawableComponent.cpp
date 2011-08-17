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
#include "CoordinateComponent.hpp"
#include "Entity.hpp"
#include "message/DrawMessage.hpp"
#include "message/LoadMessage.hpp"

#include "world/DataManager.hpp"
#include "pipeline/interface.hpp"

#include "util/util.hpp"

DrawableComponent::DrawableComponent(Entity* owner)
  : Component(owner), coord(0), skel(0)
{
  m_mesh = m_owner->m_datamgr->loadMesh("monkey");
  m_mat = m_owner->m_datamgr->loadMaterial("simple");
}

DrawableComponent::~DrawableComponent()
{}

void DrawableComponent::receiveMessage(const Message& msg)
{
  // DrawMessage
  try {
    const DrawMessage& dmsg = dynamic_cast<const DrawMessage&>(msg);
    draw(dmsg.pipe);
    return;
  } catch (std::bad_cast&) {}

  // LoadMessage
  try {
    const LoadMessage& lmsg = dynamic_cast<const LoadMessage&>(msg);
    UNUSED(lmsg);
    auto end = m_owner->m_components.end();
    for(auto i = m_owner->m_components.begin(); i != end; ++i) {
      try {
	coord = dynamic_cast<CoordinateComponent*>(*i);
	break;
      } catch (std::bad_cast&) {}
    }

    if(!coord)
      throw std::runtime_error("DrawableComponent requires CoordinateComponent");
  } catch (std::bad_cast&) {}
}

void DrawableComponent::draw(Pipeline* pipe)
{
  pipe->addDrawTask(m_mesh, m_mat, coord->transform(), Pipeline::PassStandard);
}

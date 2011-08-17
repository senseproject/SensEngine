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

#include "CoordinateComponent.hpp"
#include "Entity.hpp"

CoordinateComponent::CoordinateComponent(Entity* owner)
  : Component(owner), local2parent(1.f), local2world(1.f), parent2world(1.f)
{}

CoordinateComponent::~CoordinateComponent()
{}

void CoordinateComponent::setParentTransform(glm::mat4 par2wor)
{
  parent2world = par2wor;
  local2world = local2parent * parent2world;
  // TODO: send update message to other components
}

void CoordinateComponent::setTransform(glm::mat4 loc2par)
{
  local2parent = loc2par;
  local2world = local2parent * parent2world;
  // TODO: send update message to other components
}

void CoordinateComponent::receiveMessage(const Message&)
{}

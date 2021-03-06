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

#ifndef SENSE_ENTITY_COORDINATECOMPONENT_HPP
#define SENSE_ENTITY_COORDINATECOMPONENT_HPP

#include "3rdparty/glm/glm.hpp"

#include "Component.hpp"

class CoordinateComponent : public Component
{
public:
  CoordinateComponent(Entity*);
  virtual ~CoordinateComponent();

  void setParentTransform(glm::mat4 par2wor);
  void setTransform(glm::mat4 loc2par);

  virtual void receiveMessage(const Message&);

  const glm::mat4& transform() const { return local2world; }

private:
  glm::mat4 local2parent;
  glm::mat4 local2world;
  glm::mat4 parent2world;
};

#endif // SENSE_ENTITY_COORDINATECOMPONENT_HPP

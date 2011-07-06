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

#ifndef SENSE_ENTITY_ENTITY_HPP
#define SENSE_ENTITY_ENTITY_HPP

#include <boost/uuid/uuid.hpp>
#include <string>
#include <vector>

class Component;

struct Entity
{
  std::string m_type;
  std::string m_name;
  boost::uuids::uuid m_uuid;
  std::vector<Component*> m_components;
};

// an EntityFactory subclass creates a specific entity by
// building and attaching the needed set of components.

// A factory subclass should set Entity::m_type to a
// string value representing the logical type of the entity.

// Entity factories can have local data (hence the virtual destructor)
// However, that local data should not effect the creation of an entity
// except in trivial ways (for example, randomly selecting between multiple
// possible mesh designs)
class EntityFactory
{
public:
  virtual ~EntityFactory();
  virtual Entity* create() = 0;
  // TODO: a creation method that can read serialized entity data
};

#endif // SENSE_ENTITY_ENTITY_HPP

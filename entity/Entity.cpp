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

#include "Entity.hpp"
#include "Component.hpp"

Component::~Component()
{}

EntityFactory::~EntityFactory()
{}

Entity* EntityManager::createEntity(std::string classname)
{
  Entity* e =  m_factories[classname]->create();
  e->m_type = classname;
  e->m_uuid = m_uuidgen();
  return e;
}

void EntityManager::destroyEntity(Entity* e)
{
  for(auto i = e->m_components.begin(); i != e->m_components.end(); ++i)
    delete *i;
  delete e;
}

void EntityManager::addFactory(std::string classname, EntityFactory* fact)
{
  m_factories.insert(std::make_pair(classname, fact));
}

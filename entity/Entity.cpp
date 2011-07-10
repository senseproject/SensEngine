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

Component::Component(Entity* owner)
 : m_owner(owner)
{
  m_owner->m_components.insert(this);
}

Component::~Component()
{
  m_owner->m_components.erase(m_owner->m_components.find(this));
}

void Component::sendMessage(const Message& m)
{
  for(auto i = m_owner->m_components.begin(); i != m_owner->m_components.end(); ++i)
    if(*i != this)
      (*i)->receiveMessage(m);
}

EntityFactory::~EntityFactory()
{}

Entity* EntityManager::createEntity(std::string classname, boost::uuids::uuid* uuid)
{
  Entity* e =  m_factories[classname]->create();
  if (e) {
    e->m_type = classname;
    if(uuid)
      e->m_uuid = *uuid;
    else
      e->m_uuid = m_uuidgen();
    m_entities.insert(std::make_pair(e->m_uuid, e));
  }
  return e;
}

void EntityManager::destroyEntity(boost::uuids::uuid id)
{
  auto i = m_entities.find(id);
  if (i != m_entities.end()) {
    Entity* e = i->second;
    m_entities.erase(i);
    for(auto i = e->m_components.begin(); i != e->m_components.end(); ++i)
      delete *i;
    delete e;
  }
}

Entity* EntityManager::findEntity(boost::uuids::uuid id)
{
  auto i = m_entities.find(id);
  if (i != m_entities.end())
    return i->second;
  else
    return 0;
}

void EntityManager::addFactory(std::string classname, EntityFactory* fact)
{
  m_factories.insert(std::make_pair(classname, fact));
}

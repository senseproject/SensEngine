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

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <set>
#include <string>
#include <unordered_map>

class Component;
class CoordinateComponent;
class DrawableComponent;
class PhysicalComponent;

class DataManager;

struct Entity
{
  std::string m_type;
  std::string m_name;
  boost::uuids::uuid m_uuid;
  std::set<Component*> m_components;

  // Components that need to have "quick access" by other components
  // It's a hack, but there's really nothing to do for it.
  CoordinateComponent* m_coord;
  DrawableComponent* m_draw;
  PhysicalComponent* m_phys;

  // The high-level components needed by this Entity
  DataManager* m_datamgr;
};

// an EntityFactory subclass creates a specific entity by
// building and attaching the needed set of components.

// A factory subclass does not need to set m_type. This is handled
// by the EntityManager. The same goes for generating a new UUID
// for the Entity instance.

// Entity factories can have local data (hence the virtual destructor)
// However, that local data should not affect the creation of an entity
// except in trivial ways (for example, randomly selecting between multiple
// possible mesh designs)
class EntityFactory
{
public:
  virtual ~EntityFactory();

private:
  virtual Entity* create() = 0;
  // TODO: a creation method that can read serialized entity data

  friend class EntityManager;
};

class EntityManager
{
public:
  Entity* createEntity(std::string, boost::uuids::uuid* uuid=NULL);
  void destroyEntity(boost::uuids::uuid);
  Entity* findEntity(boost::uuids::uuid);

  void addFactory(std::string, EntityFactory*);
private:
  std::unordered_map<boost::uuids::uuid, Entity*, boost::hash<boost::uuids::uuid> > m_entities;
  std::unordered_map<std::string, EntityFactory*> m_factories;
  boost::uuids::random_generator m_uuidgen;
};

#endif // SENSE_ENTITY_ENTITY_HPP

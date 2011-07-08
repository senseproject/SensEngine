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

#ifndef SENSE_ENTITY_COMPONENT_HPP
#define SENSE_ENTITY_COMPONENT_HPP

class Entity;
class Message;

class Component
{
public:
  Component(Entity* owner);
  
  virtual ~Component();

  virtual void receiveMessage(const Message&) = 0;

protected:
  virtual void sendMessage(const Message&);

  Entity* m_owner;
};

#endif // SENSE_ENTITY_COMPONENT_HPP

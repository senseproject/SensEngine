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

#include "python/module.hpp"
#include "python/pywarnings.hpp"
#include "entity/Entity.hpp"

#include "PyEntity.hpp"

static PyObject* PyEntity_new(PyTypeObject* type, PyObject*, PyObject*)
{
  PyEntity* entity = (PyEntity*)type->tp_alloc(type, 0);
  entity->ent = new Entity;
  return (PyObject*)entity;
}

PyTypeObject PyEntity_Type = {
  PyObject_HEAD_INIT(0)
  "SensEngine.Entity",
  sizeof(PyEntity),
  0,
  0,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  Py_TPFLAGS_DEFAULT,
  "SensEngine game entity class. Does nothing without components",
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0,
  PyEntity_new
};

extern void initEntityFactory(PyObject*);
extern void initComponentClasses(PyObject*);

void initEntityClasses(PyObject* m)
{
  if(PyType_Ready(&PyEntity_Type) < 0)
    return;
  Py_INCREF(&PyEntity_Type);
  PyModule_AddObject(m, "Entity", (PyObject*)&PyEntity_Type);
  initEntityFactory(m);
  initComponentClasses(m);
}

PyObject* PyEntity_create(Entity* e)
{
  PyEntity* entity = (PyEntity*)PyEntity_Type.tp_alloc(&PyEntity_Type, 0);
  entity->ent = e;
  return (PyObject*)entity;
}

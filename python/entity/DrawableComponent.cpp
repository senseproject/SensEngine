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
#include "entity/DrawableComponent.hpp"

#include "PyEntity.hpp"

struct PyDrawableComponent
{
  PyObject_HEAD;
  DrawableComponent* coord;
};

static PyObject* PyDrawableComponent_new(PyTypeObject* type, PyObject* args, PyObject*)
{
  PyObject* owner;
  if(!PyArg_ParseTuple(args, "O", &owner))
    return NULL;

  if(!PyObject_IsInstance(owner, (PyObject*)&PyEntity_Type)) {
    PyErr_SetString(PyExc_TypeError, "Must pass an Entity object as the owner");
    return NULL;
  }
  
  PyDrawableComponent* c = (PyDrawableComponent*)type->tp_alloc(type, 0);
  c->coord = new DrawableComponent(((PyEntity*)owner)->ent);
  return (PyObject*)c;
}

extern PyTypeObject PyComponent_Type;

PyTypeObject PyDrawableComponent_Type = {
  PyObject_HEAD_INIT(0)
  "SensEngine.Components.Drawable",
  sizeof(PyDrawableComponent),
  0,
  0,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  Py_TPFLAGS_DEFAULT,
  "Component that defines pipeline parameters for this object",
  0, 0, 0, 0, 0, 0,
  0, 0, 0,
  &PyComponent_Type,
  0, 0, 0, 0, 0, 0,
  PyDrawableComponent_new
};

void initDrawableComponent(PyObject* m)
{
  if(PyType_Ready(&PyDrawableComponent_Type) < 0)
    return;
  Py_INCREF(&PyDrawableComponent_Type);
  PyModule_AddObject(m, "Drawable", (PyObject*)&PyDrawableComponent_Type);
}

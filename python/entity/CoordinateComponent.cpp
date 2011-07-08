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
#include "entity/Component.hpp"
#include "entity/CoordinateComponent.hpp"

#include "PyEntity.hpp"

struct PyCoordinateComponent
{
  PyObject_HEAD;
  CoordinateComponent* coord;
};

static PyObject* PyCoordinateComponent_new(PyTypeObject* type, PyObject* args, PyObject*)
{
  PyObject* owner;
  if(!PyArg_ParseTuple(args, "O", &owner))
    return NULL;

  if(!PyObject_IsInstance(owner, (PyObject*)&PyEntity_Type)) {
    PyErr_SetString(PyExc_TypeError, "Must pass an Entity object as the owner");
    return NULL;
  }
  
  PyCoordinateComponent* c = (PyCoordinateComponent*)type->tp_alloc(type, 0);
  c->coord = new CoordinateComponent(((PyEntity*)owner)->ent);
  return (PyObject*)c;
}

extern PyTypeObject PyComponent_Type;

PyTypeObject PyCoordinateComponent_Type = {
  PyObject_HEAD_INIT(0)
  "SensEngine.Components.Coordinate",
  sizeof(PyCoordinateComponent),
  0,
  0,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  Py_TPFLAGS_DEFAULT,
  "SensEngine game entity class. Does nothing without components",
  0, 0, 0, 0, 0, 0,
  0, 0, 0,
  &PyComponent_Type,
  0, 0, 0, 0, 0, 0,
  PyCoordinateComponent_new
};

void initCoordinateComponent(PyObject* m)
{
  if(PyType_Ready(&PyCoordinateComponent_Type) < 0)
    return;
  Py_INCREF(&PyCoordinateComponent_Type);
  PyModule_AddObject(m, "Coordinate", (PyObject*)&PyCoordinateComponent_Type);
}

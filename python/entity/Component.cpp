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

struct PyComponent
{
  PyObject_HEAD;
};

static PyObject* PyComponent_new(PyTypeObject* type, PyObject*, PyObject*)
{
  PyErr_SetString(PyExc_TypeError, "Component is an abstract class ; please instantiate a proper subclass");
  return NULL;
}

PyTypeObject PyComponent_Type = {
  PyObject_HEAD_INIT(0)
  "SensEngine.Components.Component",
  sizeof(PyComponent),
  0,
  0,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  Py_TPFLAGS_DEFAULT,
  "SensEngine game entity class. Does nothing without components",
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0,
  PyComponent_new
};

extern void initCoordinateComponent(PyObject*);
extern void initDrawableComponent(PyObject*);

void initComponentClasses(PyObject* m)
{
  if(PyType_Ready(&PyComponent_Type) < 0)
    return;
  Py_INCREF(&PyComponent_Type);
  PyObject* compmod = PyImport_AddModule("SensEngine.Components");
  PyModule_AddObject(compmod, "__builtins__", PyEval_GetBuiltins());
  PyModule_AddObject(compmod, "Component", (PyObject*)&PyComponent_Type);
  PyModule_AddObject(m, "Components", compmod);
  initCoordinateComponent(compmod);
  initDrawableComponent(compmod);
}

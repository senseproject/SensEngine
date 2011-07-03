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
#include "pipeline/interface.hpp"

#include "PyMaterialDef.hpp"
#include "PyLoader.hpp"

static PyObject * Loader_new(PyTypeObject*, PyObject*, PyObject*) 
{
  PyErr_SetString(PyExc_TypeError, "Loader objects cannot be created by python code");
  return NULL;
}

static PyObject *PyLoader_add_material(PyObject *self, PyObject *args) {
  PyMaterialDef *MaterialDef;
  PyObject *MaterialName;
  if(!PyArg_ParseTuple(args, "OU", &MaterialDef, &MaterialName))
    return 0;
  if(!PyUnicode_Check(MaterialName)) {
    PyErr_SetString(PyExc_TypeError, "The material name must be a Unicode object");
    return 0;
  }
  PyObject *bytes = PyUnicode_AsUTF8String(MaterialName);
  if(!bytes) {
    return 0;
  }
  char *val = PyBytes_AsString(bytes);
  ((PyLoader*)self)->loader->addMaterial(MaterialDef->def, val);
  Py_DECREF(bytes);
  Py_RETURN_NONE;
}

static PyMethodDef PyLoader_methods[] = {
  {"add_material", PyLoader_add_material, METH_VARARGS, "Add (or replace) a material definition"},
  {0, 0, 0, 0}
};

PyTypeObject PyLoader_Type = {
  PyObject_HEAD_INIT(0)
  "SensEngine.Loader",
  sizeof(PyLoader),
  0,
  0,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  "SensEngine data loader",
  0, 0, 0, 0, 0, 0,
  PyLoader_methods,
  0, 0, 0, 0, 0,
  0, 0, 0, 0,
  Loader_new
};

void initLoader(PyObject* m)
{
  if(PyType_Ready(&PyLoader_Type) < 0)
    return;
  Py_INCREF(&PyLoader_Type);
  PyModule_AddObject(m, "Loader", (PyObject*)&PyLoader_Type);
}

PyObject* PyLoader_create(Loader* l)
{
  PyLoader* loader = (PyLoader*)PyLoader_Type.tp_alloc(&PyLoader_Type, 0);
  loader->loader = l;
  return (PyObject*)loader;
}

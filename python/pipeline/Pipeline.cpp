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
#include "pipeline/Pipeline.hpp"

#include "PyMaterialDef.hpp"

typedef PySharedPtr<Pipeline> PyPipeline;

PyObject *PyPipeline_register_material(PyObject *self, PyObject *args) {
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
  getPyPtr<Pipeline>(self)->loader().addMaterial(std::string(val), MaterialDef->def);
  Py_DECREF(bytes);
  Py_RETURN_NONE;
}

static PyMethodDef PyPipeline_methods[] = {
  {"register_material", PyPipeline_register_material, METH_VARARGS, "Add (or replace) a material definition on this renderer object"},
  {0, 0, 0, 0}
};

template <>
PyTypeObject PyPipeline::type = {
  PyObject_HEAD_INIT(0)
  "SensEngine.Pipeline",
  sizeof(PyPipeline),
  0,
  (destructor)PyPipeline::dealloc,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  "Pipeline interface. No actual rendering interfaces are exposed, just management (like materials and cameras)",
  0, 0, 0, 0, 0, 0,
  PyPipeline_methods,
  0, 0, 0, 0, 0, 0, 0,
  PyPipeline::init,
  0,
  PyPipeline::_new
};

void initPipeline(PyObject *m) {
  if(PyType_Ready(&PyPipeline::type) < 0)
    return;
  Py_INCREF((PyObject*)&PyPipeline::type);
  PyModule_AddObject(m, "Pipeline", (PyObject*)&PyPipeline::type);
}
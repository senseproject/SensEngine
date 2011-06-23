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
#include "pipeline/DefinitionTypes.hpp"

#include "PyMaterialDef.hpp"

static void MaterialDef_dealloc(PyObject *self) {
  (&((PyMaterialDef*)self)->def)->~MaterialDef();
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *MaterialDef_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyMaterialDef *self = (PyMaterialDef*)type->tp_alloc(type, 0);
  if(self) {
    new(&(self->def)) MaterialDef;
  }
  return (PyObject*)self;
}

static PyObject *PyMaterialDef_getVert(PyMaterialDef *self, PyObject* /*closure*/) {
  return PyUnicode_FromString(self->def.shaders.vert.c_str());
}

static PyObject *PyMaterialDef_getFrag(PyMaterialDef *self, PyObject* /*closure*/) {
  return PyUnicode_FromString(self->def.shaders.frag.c_str());
}

static PyObject *PyMaterialDef_getGeom(PyMaterialDef *self, PyObject* /*closure*/) {
  return PyUnicode_FromString(self->def.shaders.geom.c_str());
}

static int PyMaterialDef_setVert(PyMaterialDef *self, PyObject *value, PyObject* /*closure*/) {
  if(value == NULL) {
    PyErr_SetString(PyExc_TypeError, "Cannot delete the vertex_shader attribute");
    return -1;
  }
  if(!PyUnicode_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "The vertex_shader attribute must be a string");
    return -1;
  }
  PyObject *bytes = PyUnicode_AsUTF8String(value);
  if(!bytes) {
    return -1;
  }
  char *val = PyBytes_AsString(bytes);
  self->def.shaders.vert = val ? val : "";
  Py_DECREF(bytes);
  return 0;
}

static int PyMaterialDef_setFrag(PyMaterialDef *self, PyObject *value, PyObject* /*closure*/) {
  if(value == NULL) {
    PyErr_SetString(PyExc_TypeError, "Cannot delete the fragment_shader attribute");
    return -1;
  }
  if(!PyUnicode_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "The fragment_shader attribute must be a string");
    return -1;
  }
  PyObject *bytes = PyUnicode_AsUTF8String(value);
  if(!bytes) {
    return -1;
  }
  char *val = PyBytes_AsString(bytes);
  self->def.shaders.frag = val ? val : "";
  Py_DECREF(bytes);
  return 0;
}

static int PyMaterialDef_setGeom(PyMaterialDef *self, PyObject *value, PyObject* /*closure*/) {
  if(value == NULL) {
    PyErr_SetString(PyExc_TypeError, "Cannot delete the geometry_shader attribute");
    return -1;
  }
  if(!PyUnicode_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "The geometry_shader attribute must be a string");
    return -1;
  }
  PyObject *bytes = PyUnicode_AsUTF8String(value);
  if(!bytes) {
    return -1;
  }
  char *val = PyBytes_AsString(bytes);
  self->def.shaders.geom = val ? val : "";
  Py_DECREF(bytes);
  return 0;
}

static PyObject *PyMaterialDef_add_uniform(PyMaterialDef *self, PyObject *args, PyObject *kwds) {
  static char* keywords[] = { "name", "type", "value", 0 };
  PyObject *name;
  unsigned int itype;
  PyObject *value;
  if(!PyArg_ParseTupleAndKeywords(args, kwds, "UI|O", keywords, &name, &itype, &value))
    return 0;
  PyObject *bytes = PyUnicode_AsUTF8String(name);
  if(!bytes)
    return 0;
  char *cname = PyBytes_AsString(bytes);
  std::string sname = cname ? cname : "";
  Py_DECREF(bytes);

  UniformDef def;
  def.type = UniformDef::Type(itype);
  switch(def.type) {
    case UniformDef::Texture: {
      PyObject *bytes = PyUnicode_AsUTF8String(value);
      if(!bytes)
        return 0;
      char *cvalue = PyBytes_AsString(bytes);
      std::string svalue = cvalue ? cvalue : "";
      def.value = svalue;
      Py_DECREF(bytes);
      break;
    }
    default:
      break;
  }
  self->def.uniforms.insert(std::make_pair(sname, def));
  Py_RETURN_NONE;
}

static PyGetSetDef PyMaterialDef_getsetters[] = {
  { "vertex_shader", (getter)PyMaterialDef_getVert, (setter)PyMaterialDef_setVert, "Vertex shader to be used by this material", NULL },
  { "fragment_shader", (getter)PyMaterialDef_getFrag, (setter)PyMaterialDef_setFrag, "Fragment shader to be used by this material", NULL },
  { "geometry_shader", (getter)PyMaterialDef_getGeom, (setter)PyMaterialDef_setGeom, "Geometry shader to be used by this material", NULL },
  {NULL}
};

static PyMethodDef PyMaterialDef_methods[] = {
  {"add_uniform", (PyCFunction)PyMaterialDef_add_uniform, METH_VARARGS|METH_KEYWORDS, "Add uniform data to this material" },
  {0, 0, 0, 0}
};

PyTypeObject PyMaterialDef_Type = {
  PyObject_HEAD_INIT(0)
  "SensEngine.MaterialDef",
  sizeof(PyMaterialDef),
  0,
  MaterialDef_dealloc,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  "Definition information for a SensEngine material",
  0, 0, 0, 0, 0, 0,
  PyMaterialDef_methods,
  0,
  PyMaterialDef_getsetters,
  0, 0, 0, 0, 0,
  0, 0,
  MaterialDef_new
};

void initMaterialDefinition(PyObject *m) {
  if(PyType_Ready(&PyMaterialDef_Type) < 0)
    return;
  Py_INCREF(&PyMaterialDef_Type);
  PyModule_AddObject(m, "MaterialDef", (PyObject*)&PyMaterialDef_Type);

  // Add all the special constants to our material type
  PyDict_AddEnum(PyMaterialDef_Type.tp_dict, UniformDef, Texture);
  PyDict_AddEnum(PyMaterialDef_Type.tp_dict, UniformDef, Webview);
  PyDict_AddEnum(PyMaterialDef_Type.tp_dict, UniformDef, ModelView);
  PyDict_AddEnum(PyMaterialDef_Type.tp_dict, UniformDef, Projection);
  PyDict_AddEnum(PyMaterialDef_Type.tp_dict, UniformDef, DepthInfo);
  PyDict_AddEnum(PyMaterialDef_Type.tp_dict, UniformDef, LightPosition);
  PyDict_AddEnum(PyMaterialDef_Type.tp_dict, UniformDef, LightColor);
  PyDict_AddEnum(PyMaterialDef_Type.tp_dict, UniformDef, LightRadius);
  PyDict_AddEnum(PyMaterialDef_Type.tp_dict, UniformDef, BoneMatrices);
}

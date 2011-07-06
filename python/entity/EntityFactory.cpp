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

class PythonEntityFactory : public EntityFactory
{
public:
  PythonEntityFactory(PyObject* self)
    : m_self(self)
    {
      Py_INCREF(m_self);
    }
      
  ~PythonEntityFactory()
    {
      Py_DECREF(m_self);
    }

private:
  
  virtual Entity* create()
    {
      // TODO: pass a default set of arguments
      PyObject_CallMethod(m_self, "create", 0);
      return new Entity; // TODO: parse the actual result
    }

  // TODO: some sort of serializaton read function

  PyObject* m_self;
};

struct PyEntityFactory
{
  PyObject_HEAD;
  PythonEntityFactory* fact;
};

struct PyEntityManager
{
  PyObject_HEAD;
  EntityManager* mgr;
};

static PyObject* PyEntityFactory_new(PyTypeObject* type, PyObject*, PyObject*)
{
  PyEntityFactory *factory = (PyEntityFactory*)type->tp_alloc(type, 0);
  PythonEntityFactory *fact = new PythonEntityFactory((PyObject*)factory);
  factory->fact = fact;
  return (PyObject*)factory;
}

static PyObject* PyEntityManager_new(PyTypeObject*, PyObject*, PyObject*)
{
  PyErr_SetString(PyExc_TypeError, "EntityManager objects cannot be created by python code");
  return NULL;
}

static PyObject* PyEntityManager_add_factory(PyObject* self, PyObject* args)
{
  PyObject* factory;
  PyObject* classname;
  if(!PyArg_ParseTuple(args, "OU", &factory, &classname))
    return 0;


  if(!PyUnicode_Check(classname)) {
    PyErr_SetString(PyExc_TypeError, "Class name must be a unicode object");
    return 0;
  }

  PyObject* bytes = PyUnicode_AsUTF8String(classname);
  if(!bytes)
    return 0;

  char* classstring = PyBytes_AsString(bytes);
  PythonEntityFactory* fact = new PythonEntityFactory(factory);
  ((PyEntityManager*)self)->mgr->addFactory(classstring, fact);
  Py_DECREF(bytes);
  Py_RETURN_NONE;
}

static PyMethodDef PyEntityManager_methods[] = {
  {"add_factory", PyEntityManager_add_factory, METH_VARARGS, "Add a new entity factory for the given classname"},
  {0, 0, 0, 0}
};

PyTypeObject PyEntityManager_Type = {
  PyObject_HEAD_INIT(0)
  "SensEngine.EntityManager",
  sizeof(PyEntityManager),
  0,
  0,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  Py_TPFLAGS_DEFAULT,
  "SensEngine Entity management",
  0, 0, 0, 0, 0, 0,
  PyEntityManager_methods,
  0, 0, 0, 0, 0,
  0, 0, 0, 0,
  PyEntityManager_new
};

PyTypeObject PyEntityFactory_Type = {
  PyObject_HEAD_INIT(0)
  "SensEngine.EntityFactory",
  sizeof(PyEntityFactory),
  0,
  0,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  "SensEngine Entity factory class",
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0,
  PyEntityFactory_new
};

void initEntityFactory(PyObject* m)
{
  if(PyType_Ready(&PyEntityFactory_Type) < 0)
    return;
  Py_INCREF(&PyEntityManager_Type);
  PyModule_AddObject(m, "EntityFactory", (PyObject*)&PyEntityFactory_Type);

  if(PyType_Ready(&PyEntityManager_Type) < 0)
    return;
  Py_INCREF(&PyEntityManager_Type);
  PyModule_AddObject(m, "EntityManager", (PyObject*)&PyEntityManager_Type);
}

PyObject* PyEntityManager_create(EntityManager* m)
{
  PyEntityManager* mgr = (PyEntityManager*)PyEntityManager_Type.tp_alloc(&PyEntityManager_Type, 0);
  mgr->mgr = m;
  return (PyObject*)mgr;
}

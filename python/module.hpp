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

#ifndef SENSE_PYTHON_MODULE_HPP
#define SENSE_PYTHON_MODULE_HPP

#include <Python.h>
#include <memory>
#include <stdexcept>
#include <string>

template<typename T>
struct PySharedPtr {
  PyObject_HEAD;
  std::shared_ptr<T> ptr;
  static PyTypeObject type;
  
  static void dealloc(PySharedPtr<T>* self) {
    (&(self->ptr))->~shared_ptr();
    Py_TYPE(self)->tp_free((PyObject*)self);
  }
  
  static PyObject* _new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/) {
    PySharedPtr<T>* self;
    self = (PySharedPtr<T>*)type->tp_alloc(type, 0);
    new(&(self->ptr)) std::shared_ptr<T>();
    try {
      self->ptr = std::shared_ptr<T>(new T);
    } catch (const std::runtime_error& err) {
      PyErr_SetString(PyExc_RuntimeError, err.what());
      type->tp_free((PyObject*)self);
      return 0;
    }
    return (PyObject*)self;
  }
    
  // Not all classes have to use this init, but its a solid template for an empty init function
  static int init(PyObject* /*self*/, PyObject* args, PyObject* kwds) {
    static char* kwlist[] = { NULL };
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist))
      return -1;
    return 0;
  }
};

template<typename T>
struct PySharedPtrAbstract {
  PyObject_HEAD;
  std::shared_ptr<T> ptr;
  static PyTypeObject type;
  
  static void dealloc(PySharedPtr<T>* self) {
    (&(self->ptr))->~shared_ptr();
    Py_TYPE(self)->tp_free((PyObject*)self);
  }
  
  static PyObject* _new(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/) {
    PyErr_SetString(PyExc_TypeError, "Abstract class type");
    return 0;
  }
    
  // Not all classes have to use this init, but its a solid template for an empty init function
  static int init(PyObject* /*self*/, PyObject* args, PyObject* kwds) {
    static char* kwlist[] = { NULL };
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist))
      return -1;
    return 0;
  }
};

template<typename T>
PyObject* createPyObject(std::shared_ptr<T> ptr) {
  PyTypeObject* type = &PySharedPtr<T>::type;
  if(!ptr.get())
    Py_RETURN_NONE;
  Py_INCREF((PyObject*)&PySharedPtr<T>::type);
  PySharedPtr<T> *obj = (PySharedPtr<T>*)type->tp_alloc(type, 0);
  new(&(obj->ptr)) std::shared_ptr<T>(ptr);
  return (PyObject*)obj;
}

template<typename T>
std::shared_ptr<T> getPyPtr(PyObject* obj) {
  if(!PyObject_IsInstance(obj, (PyObject*)&PySharedPtr<T>::type))
    return std::shared_ptr<T>();
  else
    return ((PySharedPtr<T>*)obj)->ptr;
}

template<typename T>
std::shared_ptr<T> getAbstractPyPtr(PyObject* obj) {
  if(!PyObject_IsInstance(obj, (PyObject*)&PySharedPtrAbstract<T>::type))
    return std::shared_ptr<T>();
  else
    return ((PySharedPtr<T>*)obj)->ptr;
}

PyMODINIT_FUNC initSensEngine(void);
void restoreSysPath(PyObject*);
PyObject* appendSysPath(const char*, bool=false);

#define PyDict_AddEnum(dict, enum, value) PyDict_SetItemString(dict, #value, PyLong_FromLong(enum::value));

#endif // SENSE_PYTHON_MODULE_HPP

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
  static int init(PySharedPtr<T>* /*self*/, PyObject* args, PyObject* kwds) {
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
  static int init(PySharedPtr<T>* /*self*/, PyObject* args, PyObject* kwds) {
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

#endif // SENSE_PYTHON_MODULE_HPP

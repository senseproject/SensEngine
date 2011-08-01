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

#include "module.hpp"
#include "pywarnings.hpp"

extern void initMaterialDefinition(PyObject*);
extern void initDataManager(PyObject*);
extern void initEntityClasses(PyObject*);

PyModuleDef SenseModule = {
  PyModuleDef_HEAD_INIT,
  "SensEngine",
  "The SensEngine python bindings. These bindings are available both for in-engine scripting and as a standalone python module.",
  -1, 0, 0, 0, 0, 0
};

PyMODINIT_FUNC initSensEngine() {
  PyObject *m = PyModule_Create(&SenseModule);
  PyModule_AddStringConstant(m, "__path__", "SensEngine");
  initMaterialDefinition(m);
  initDataManager(m);
  initEntityClasses(m);
  return m;
}

PyObject* appendSysPath(const char* path, bool save_old) {
  PyObject* sys_path = PySys_GetObject("path");
  PyObject* old_sys_path = 0;
  if(save_old) {
     old_sys_path = PySequence_List(sys_path);
  }
  PyObject* path_as_pyunicode = PyUnicode_FromString(path);
  PyList_Append(sys_path, path_as_pyunicode);
  Py_DECREF(path_as_pyunicode);
  return old_sys_path;
}


void restoreSysPath(PyObject* sys_path) {
  PySys_SetObject("path", sys_path);
  Py_DECREF(sys_path);
}

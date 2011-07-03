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

PyMODINIT_FUNC initSensEngine(void);
void restoreSysPath(PyObject*);
PyObject* appendSysPath(const char*, bool=false);

#define PyDict_AddEnum(dict, enum, value) PyDict_SetItemString(dict, #value, PyLong_FromLong(enum::value));

#endif // SENSE_PYTHON_MODULE_HPP

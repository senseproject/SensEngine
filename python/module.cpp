#include "module.hpp"
#include "pywarnings.hpp"

PyModuleDef SenseModule = {
  PyModuleDef_HEAD_INIT,
  "SensEngine",
  "The SensEngine python bindings. These bindings are available both for in-engine scripting and as a standalone python module.",
  -1, 0, 0, 0, 0, 0
};

PyMODINIT_FUNC initSensEngine() {
  PyObject *m = PyModule_Create(&SenseModule);
  return m;
}

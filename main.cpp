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
#include "pipeline/Pipeline.hpp"

void initClientModule(std::shared_ptr<Pipeline> p) {
  PyObject* SensModule = PyImport_ImportModule("SensEngine");
  PyObject* m = PyImport_AddModule("SensEngine.client");
  PyModule_AddObject(m, "__builtins__", PyEval_GetBuiltins());
  PyModule_AddObject(m, "pipeline", createPyObject(p));
  PyModule_AddObject(SensModule, "client", m);
  PyModule_AddStringConstant(SensModule, "__path__", "SensEngine");
}

struct DemoConsole : public Pipeline::KbdCallback {
  virtual ~DemoConsole() {}
  void operator()(Keysym, uint32_t) {}
};

int main(int argc, char **argv) {
  std::shared_ptr<Pipeline> p = std::shared_ptr<Pipeline>(new Pipeline);
  p->pushKbdCallback(new DemoConsole);
  auto fsaa_level_iter = p->fsaaLevels().rbegin();
  p->setFsaa(*fsaa_level_iter); // set the highest possible FSAA level

  Py_NoSiteFlag = 1;
  Py_SetProgramName((wchar_t*)L"SensEngine");
  Py_InitializeEx(0);
  PyImport_AppendInittab("SensEngine", initSensEngine);
  initClientModule(p);
  p->loader().loadMaterialFiles("../data/materials");
  Pipeline::hPanel hud = p->createPanel(0.5f, 0.5f, "gui_hud");
  for(;;) {
    p->beginFrame();
    p->render();
    p->endFrame();
    if(!p->platformEventLoop())
      break;
  }
  p->destroyPanel(hud);
}

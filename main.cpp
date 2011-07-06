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
#include "client/Client.hpp"

// #include <berkelium/Berkelium.hpp>

void initClientModule() {

}

int main(int argc, char **argv) {
//   if(!Berkelium::init(Berkelium::FileString::empty()))
//     throw std::runtime_error("Could not initialize Berkelium!");

  Py_NoSiteFlag = 1;
  Py_SetProgramName((wchar_t*)L"SensEngine");
  Py_InitializeEx(0);
  PyImport_AppendInittab("SensEngine", initSensEngine);

  SenseClient client;

  for(;;) {
//     Berkelium::update();
    if(!client.tick())
      break;
  }
}

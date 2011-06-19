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

#include "pipeline/Pipeline.hpp"

struct DemoConsole : public Pipeline::KbdCallback {
  virtual ~DemoConsole() {}
  void operator()(Keysym, uint32_t) {}
};

int main(int argc, char **argv) {
  std::shared_ptr<Pipeline> p = std::shared_ptr<Pipeline>(new Pipeline);
  p->pushKbdCallback(new DemoConsole);
  auto fsaa_level_iter = p->fsaaLevels().rbegin();
  p->setFsaa(*fsaa_level_iter); // set the highest possible FSAA level
  for(;;) {
    p->beginFrame();
    p->render();
    p->endFrame();
    if(!p->platformEventLoop())
      break;
  }
}

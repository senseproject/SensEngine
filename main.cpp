#include "pipeline/Pipeline.hpp"

struct DemoConsole : public Pipeline::KbdCallback {
  virtual ~DemoConsole() {}
  void operator()(Keysym, uint32_t) {}
};

int main(int argc, char **argv) {
  Pipeline p;
  p.pushKbdCallback(new DemoConsole);
  auto fsaa_level_iter = p.fsaaLevels().rbegin();
  p.setFsaa(*fsaa_level_iter); // set the highest possible FSAA level
  for(;;) {
    p.beginFrame();
    p.render();
    p.endFrame();
    if(!p.platformEventLoop())
      break;
  }
}

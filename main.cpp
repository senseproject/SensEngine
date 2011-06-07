#include "pipeline/Pipeline.hpp"

struct DemoConsole : public Pipeline::KbdCallback {
  virtual ~DemoConsole() {}
  void operator()(Keysym, uint32_t) {}
};

int main(int argc, char **argv) {
  Pipeline p;
  p.pushKbdCallback(new DemoConsole);
  p.setFsaa(*(p.fsaaLevels().rbegin())); // set the highest possible FSAA level
  for(;;) {
    p.beginFrame();
    p.render();
    p.endFrame();
    if(!p.platformEventLoop())
      break;
  }
}

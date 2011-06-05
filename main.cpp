#include "Pipeline.hpp"

struct DemoConsole : public Pipeline::KbdCallback {
  virtual ~DemoConsole() {}
  void operator()(Keysym, uint32_t) {}
};

int main(int argc, char **argv) {
  Pipeline p;
  p.pushKbdCallback(new DemoConsole);
  for(;;) {
    p.beginFrame();
    p.render();
    p.endFrame();
    if(!p.platformEventLoop())
      break;
  }
}

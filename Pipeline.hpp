#include <cstdint>
#include <thread>

struct PipelinePlatform;

class Pipeline {
public:
  struct KbdCallback {
    enum Keysym {
    };
    virtual void operator()(Keysym, uint32_t) =0;
  };

private:
  PipelinePlatform *platform;
  std::thread loader_thread;
  bool loader_init_complete;
  bool loader_init_error;
  std::string loader_error_string;

  void platformInit();
  void platformInitLoader();
  void platformFinish();
  void platformSwap();
  void platformDetachContext();
  void platformAttachContext();

protected:
  void runLoaderThread();

public:
  Pipeline();
  ~Pipeline();

  void pushKbdCallback(KbdCallback*);

  void beginFrame();
  void endFrame();

  friend void launchLoaderThread(Pipeline*);
};
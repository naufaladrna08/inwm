#include <cstdio>
#include <cstdlib>
#include <memory>
// #include "Logging.hpp"
#include "WindowManager.hpp"

int main(int argc, char** argv) {
  std::unique_ptr<WindowManager> wm(WindowManager::Create());
  if (!wm) {
    printf("Unable to initialize window manager\n");
    return -1;
  }

  wm->Run();

  return 0;
}

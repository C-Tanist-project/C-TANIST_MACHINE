#include <thread>

#include "ui.hpp"
#include "vm.hpp"

int main() {
  Operations::InitializeMap();

  VMState *vm = VMStateSetup();

  std::thread engine(VMEngine, vm);

  GLFWwindow *window = MainWindowSetup(1280, 720, "Pentacle VM");

  IMGUIsetup(window);

  while (!glfwWindowShouldClose(window)) {
    std::unique_lock lock(vm->mutex);
    RenderMainWindow(window);
    lock.unlock();
  }
  engine.join();
  WindowCleanup(window);

  return 0;
}

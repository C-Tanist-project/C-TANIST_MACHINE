#include "ui.hpp"
#include "vm.hpp"

int main() {
  GLFWwindow *window = MainWindowSetup(1280, 720, "Pentacle VM");

  IMGUIsetup(window);
  VMState *vm_state = new VMState;

  while (!glfwWindowShouldClose(window)) {
    RenderMainWindow(window, vm_state);
  }

  WindowCleanup(window);

  return 0;
}

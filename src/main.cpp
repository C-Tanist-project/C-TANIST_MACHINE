#include "ui.hpp"
#include "vm.hpp"

int main() {
  Operations::InitializeMap();

  GLFWwindow *window = MainWindowSetup(1280, 720, "Pentacle VM");

  IMGUIsetup(window);

  while (!glfwWindowShouldClose(window)) {
    RenderMainWindow(window);
  }

  WindowCleanup(window);

  return 0;
}

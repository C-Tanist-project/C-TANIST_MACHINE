#include <fstream>
#include <iostream>
#include <thread>

#ifdef _WIN32
setlocale(LC_ALL, ".UTF8");
#endif

#include "ui.hpp"
#include "vm.hpp"

int main(int argc, char *argv[]) {
  Operations::InitializeMap();
  VMEngine engine;
  VMState vm;

  VMStateSetup(vm);

  std::ifstream file(argv[1], std::ios::binary);

  if (!file) {
    std::cerr << "Error opening file!\n";
    return 1;
  }

  int16_t instructionsMemoryAddress = 32;

  int16_t buffer;

  while (file.read(reinterpret_cast<char *>(&buffer), sizeof(buffer))) {
    vm.memory[instructionsMemoryAddress] = buffer;
    ++instructionsMemoryAddress;
  }

  std::thread engineThread(&VMEngine::Run, &engine, std::ref(vm));

  GLFWwindow *window = MainWindowSetup(1280, 720, "Pentacle VM");

  IMGUIsetup(window);

  for (int i = 31; i < 50; ++i) {
    std::cout << vm.memory[i] << " ";
  }

  std::cout << std::endl;

  SetupCtanistStyle();

  while (!glfwWindowShouldClose(window)) {
    RenderMainWindow(window, vm);
  }

  VMEngine::NotifyCommand(CLOSE);
  engineThread.join();

  WindowCleanup(window);

  return 0;
}

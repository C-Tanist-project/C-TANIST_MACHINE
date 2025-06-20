#include <fstream>
#include <iostream>

#include "ui.hpp"
#include "vm.hpp"

int main(int argc, char* argv[]) {
  std::ifstream file(argv[1], std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file!\n";
    return 1;
  }

  VMState* vm = new VMState;
  int16_t instructionsMemoryAddress = vm->pc;

  unsigned char buffer[2];

  while (file.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
    u_int16_t word = (buffer[0] << 8) | (buffer[1]);
    vm->memory[instructionsMemoryAddress] = word;
    ++instructionsMemoryAddress;
  }

  for (int i = 31; i < 50; ++i) {
    std::cout << vm->memory[i] << " ";
  }

  GLFWwindow* window = MainWindowSetup(1280, 720, "Pentacle VM");

  IMGUIsetup(window);

  while (!glfwWindowShouldClose(window)) {
    RenderMainWindow(window);
  }

  WindowCleanup(window);

  return 0;
}

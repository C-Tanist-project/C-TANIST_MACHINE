#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "assembler.hpp"
// #include "ui.hpp"
// #include "vm.hpp"

int main(int argc, char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "assemble") == 0) {
    std::cout << "CWD = " << std::filesystem::current_path() << '\n';
    std::string test = "tests/testFirstPass.asm";

    printf("Assembling %s\n", test.c_str());
    std::string objFilePath = "tests/testFirstPass.obj";
    std::string lstFilePath = "tests/testFirstPass.lst";

    Assembler *assembler = new Assembler(test, objFilePath, lstFilePath);
    AssemblerExitCode exitCode = assembler->Assemble();

    std::cout << "Exit code: " << exitCode << std::endl;

    for (auto &[symbol, data] : assembler->symbolTable) {
      std::cout << "Symbol: " << symbol << ", Address: " << data.address
                << ", Defined: " << (data.defined ? "Yes" : "No") << std::endl;
    }

    for (auto &[literal, data] : assembler->literalTable) {
      std::cout << "Literal: " << literal << ", Address: " << data.address
                << ", Defined: " << (data.defined ? "Yes" : "No") << std::endl;
    }

    for (const auto &[symbol, data] : assembler->intDefTable) {
      std::cout << "IntDef Symbol: " << symbol << ", Address: " << data.address
                << ", Defined: " << (data.defined ? "Yes" : "No") << std::endl;
    }

    for (const auto &[symbol, data] : assembler->intUseTable) {
      std::cout << "IntUse Symbol: " << symbol << " -> [ ";
      for (const auto &elem : data) {
        std::cout << elem << " ";
      }
      std::cout << "]" << std::endl;
    }
    return 0;
  }

  // Operations::InitializeMap();
  // VMEngine engine;
  // VMState vm;

  // VMStateSetup(vm);

  // std::ifstream file(argv[1], std::ios::binary);

  // if (!file) {
  //   std::cerr << "Error opening file!\n";
  //   return 1;
  // }

  // int16_t instructionsMemoryAddress = 32;

  // int16_t buffer;

  // while (file.read(reinterpret_cast<char *>(&buffer), sizeof(buffer))) {
  //   vm.memory[instructionsMemoryAddress] = buffer;
  //   ++instructionsMemoryAddress;
  // }

  // std::thread engineThread(&VMEngine::Run, &engine, std::ref(vm));

  // GLFWwindow *window = MainWindowSetup(1280, 720, "Pentacle VM");

  // IMGUIsetup(window);

  // for (int i = 31; i < 50; ++i) {
  //   std::cout << vm.memory[i] << " ";
  // }

  // std::cout << std::endl;

  // SetupCtanistStyle();

  // while (!glfwWindowShouldClose(window)) {
  //   RenderMainWindow(window, vm);
  // }

  // VMEngine::NotifyCommand(CLOSE);
  // engineThread.join();

  // WindowCleanup(window);

  // return 0;
}

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "assembler.hpp"
#include "linker.hpp"
#include "ui.hpp"
#include "vm.hpp"

int main(int argc, char *argv[]) {
  Operations::InitializeMap();
  VMEngine engine;
  VMState vm;

  VMStateSetup(vm);
  if (argc < 2) {
    std::cerr << "Não há arquivos para montar" << std::endl;
    return 1;  // Eventualmente isso deverá ser tratado como uma exception
  }

  Assembler *assembler = new Assembler();

  std::vector<std::string> objFiles;

    for (int i = 0; i < argc; ++i) {
    std::string fileName = argv[i];
    std::string asmFilePath = "./asm/" + fileName;
    objFiles.push_back(asmFilePath);
    std::string baseName = fileName.substr(0, fileName.find_last_of('.'));
    std::string objFilePath = "./obj/" + baseName + ".obj";
    std::string lstFilePath = "./lst/" + baseName + ".lst";

    assembler->Assemble(asmFilePath, objFilePath, lstFilePath);
  }

  Linker *linker = new Linker;
  linker->ReadObjectCodeFile(objFiles[0]);
  linker->printModules();

  // int16_t instructionsMemoryAddress = 32;

  // int16_t buffer;

  // while (file.read(reinterpret_cast<char *>(&buffer), sizeof(buffer))) {
  //   vm.memory[instructionsMemoryAddress] = buffer;
  //   ++instructionsMemoryAddress;
  // }

  std::thread engineThread(&VMEngine::Run, &engine, std::ref(vm));

  GLFWwindow *window = MainWindowSetup(1280, 720, "Pentacle VM");

  IMGUIsetup(window);

  SetupCtanistStyle();

  while (!glfwWindowShouldClose(window)) {
    RenderMainWindow(window, vm);
  }

  VMEngine::NotifyCommand(CLOSE);
  engineThread.join();

  WindowCleanup(window);
  return 0;
}

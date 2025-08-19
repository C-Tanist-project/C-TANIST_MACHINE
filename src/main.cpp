#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "assembler.hpp"
#include "linker.hpp"
#include "preprocessor.hpp"
#include "ui.hpp"
#include "vm.hpp"

int main(int argc, char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "preprocess") == 0) {
    MacroProcessor macross(argv[2]);
    macross.Pass();
    return 0;
  }
  Operations::InitializeMap();
  VMEngine engine;
  VMState vm;

  VMStateSetup(vm);

  Assembler *assembler = new Assembler();
  std::vector<std::string> paths;
  paths.push_back("./tests/test.asm");
  paths.push_back("./tests/test2.asm");
  assembler->NotifyAssembling(paths);

  Linker *linker = new Linker;

  std::vector<std::string> objPaths;
  objPaths.push_back("./obj/test.obj");
  objPaths.push_back("./obj/test2.obj");
  linker->FirstPass(objPaths);

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

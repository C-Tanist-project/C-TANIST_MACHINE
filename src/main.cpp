#include <cstring>
#include <thread>

#include "pipeline.hpp"
#include "ui.hpp"
#include "vm.hpp"

int main(int argc, char *argv[]) {
  if (argc > 2 && strcmp(argv[1], "-C") == 0) {
    AssemblyPipeline pipeline("new-project");
    for (int i = 2; i < argc; ++i) {
      pipeline.AddRawSource(argv[i]);
      pipeline.Pass();
    }
    return 0;
  }

  Operations::InitializeMap();
  VMEngine engine;
  VMState vm;
  VMStateSetup(vm);

  Linker *linker = new Linker();

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

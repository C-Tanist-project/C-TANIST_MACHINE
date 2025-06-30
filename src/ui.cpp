#include "ui.hpp"

#include "external/codicon_hex.h"
#include "vm.hpp"

// INSERIR OS WIDGETS DE VOCÊS AQUI!!!
void RenderMainWindow(GLFWwindow *window, VMState &vm) {
  glfwPollEvents();

  ImGui_ImplOpenGL3_NewFrame();

  ImGui_ImplGlfw_NewFrame();

  ImGui::NewFrame();

  // Renderização começa aqui
  static bool showControlWindow = true;

  RenderControlsWindow(showControlWindow, vm);
  RenderMemoryEditor(vm);
  RenderVMState(vm);
  RenderConsoleWindow(vm);

  ImGui::Render();

  int displayWidth, displayHeight;

  glfwGetFramebufferSize(window, &displayWidth, &displayHeight);

  glViewport(0, 0, displayWidth, displayHeight);

  glClearColor(0.0f, 0.0f, 0.0f, 1.00f);

  glClear(GL_COLOR_BUFFER_BIT);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(window);
}

GLFWwindow *MainWindowSetup(const int width, const int height,
                            const char *title) {
  glfwSetErrorCallback(GLFWErrorCallback);

  if (!glfwInit()) {
    perror("GLFW Error callback failed.");
    exit(EXIT_FAILURE);
  }

  GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (window == nullptr) {
    perror("GLFW Window Setup failed.");
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);

  glfwSwapInterval(1);  // vsync

  return window;
}

void GLFWErrorCallback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void IMGUIsetup(GLFWwindow *window) {
  const char *glsl_version = "#version 130";

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();

  (void)io;

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.Fonts->AddFontDefault();

  ImFontConfig iconsConfig;
  iconsConfig.MergeMode = true;
  iconsConfig.PixelSnapH = true;
  iconsConfig.GlyphOffset.y = 4.0f;
  iconsConfig.GlyphMinAdvanceX = 14.0f;
  iconsConfig.FontDataOwnedByAtlas = false;

  static const ImWchar iconRanges[] = {ICON_MIN_CI, ICON_MAX_CI, 0};

  io.Fonts->AddFontFromMemoryTTF(codicon_ttf, codicon_ttf_len, 15.0f,
                                 &iconsConfig, iconRanges);

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);

  ImGui_ImplOpenGL3_Init(glsl_version);
}

void WindowCleanup(GLFWwindow *window) {
  ImGui_ImplOpenGL3_Shutdown();

  ImGui_ImplGlfw_Shutdown();

  ImGui::DestroyContext();

  glfwDestroyWindow(window);

  glfwTerminate();
}

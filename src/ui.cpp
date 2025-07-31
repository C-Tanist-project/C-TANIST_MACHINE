#include "ui.hpp"

#include "external/codicon_hex.h"
#include "types.hpp"

// INSERIR OS WIDGETS DE VOCÊS AQUI!!!
void RenderMainWindow(GLFWwindow *window, VMState &vm) {
  glfwPollEvents();

  ImGui_ImplOpenGL3_NewFrame();

  ImGui_ImplGlfw_NewFrame();

  ImGui::NewFrame();
  // Renderização começa aqui
  static bool showControlWindow = false;
  static bool showMemoryEditorWindow = false;
  static bool showVMStateWindow = false;
  static bool showConsoleWindow = false;
  static bool showTextWindow = false;

  // Menu de contexto para abrir ou fechar as janelas
  if (ImGui::BeginPopupContextVoid("MenuJanelas")) {
    ImGui::MenuItem("Controles da VM", NULL, &showControlWindow);
    ImGui::MenuItem("Editor de Memória", NULL, &showMemoryEditorWindow);
    ImGui::MenuItem("Registradores", NULL, &showVMStateWindow);
    ImGui::MenuItem("Console", NULL, &showConsoleWindow);
    ImGui::MenuItem("Editor de Texto", NULL, &showTextWindow);
    ImGui::EndPopup();
  }

  RenderControlsWindow(showControlWindow, vm);

  RenderMemoryEditor(vm, showMemoryEditorWindow);

  RenderVMState(vm, showVMStateWindow);

  RenderConsoleWindow(vm, showConsoleWindow);

  RenderTextEditor(showTextWindow);

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

// altera o esquema de cores global do imgui
void SetupCtanistStyle() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  colors[ImGuiCol_Text] = ImVec4(1.00f, 0.85f, 0.85f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.25f, 0.25f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.05f, 0.05f, 1.00f);
  colors[ImGuiCol_Border] = ImVec4(0.40f, 0.00f, 0.00f, 0.50f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.45f, 0.05f, 0.05f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.70f, 0.15f, 0.15f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.30f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.45f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.00f, 0.00f, 0.60f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.00f, 0.00f, 0.80f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.55f, 0.10f, 0.10f, 0.90f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.75f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.40f, 0.40f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.90f, 0.40f, 0.40f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.55f, 0.55f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.40f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.60f, 0.10f, 0.10f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.45f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.65f, 0.10f, 0.10f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.85f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.00f, 0.00f, 0.70f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.80f, 0.20f, 0.20f, 0.90f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.40f, 0.40f, 1.00f);
  colors[ImGuiCol_Tab] = ImVec4(0.30f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.50f, 0.10f, 0.10f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.70f, 0.15f, 0.15f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.35f, 0.00f, 0.00f, 1.00f);

  style.FrameRounding = 4.0f;
  style.GrabRounding = 4.0f;
  style.WindowRounding = 2.0f;
  style.ScrollbarRounding = 3.0f;
  style.FramePadding = ImVec2(6, 4);
}

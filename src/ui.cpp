#include "ui.hpp"

// INSERIR OS WIDGETS DE VOCÊS AQUI!!!
void RenderMainWindow(GLFWwindow *window) {
  glfwPollEvents();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // EDITOR DE MEMÓRIA
  static MemoryEditor mem_edit;
  mem_edit.PreviewDataType = ImGuiDataType_U16;

  // dados de teste para ver na memória (atualizados automaticamente)
  // buffer são os dados que são atualizados pelo usuário na interface
  // os dados são salvos ao apertar o botão save
  static uint16_t buffer[500], data[500];
  size_t data_size = sizeof(data);
  size_t buffer_size = sizeof(buffer);

  static bool initialized = false;
  if (!initialized) {
    for (int i = 0; i < 500; ++i) buffer[i] = (uint16_t)0;
    memcpy(data, buffer, data_size);
    initialized = true;
  }

  mem_edit.OptShowOptions = false;
  mem_edit.OptShowDataPreview = true;

  if (mem_edit.Open) {
    ImGui::Begin("Memory Editor", &mem_edit.Open);
    mem_edit.DrawContents(buffer, buffer_size);

    // testar se buffer e data estão funcionando como previsto:
    /*
    ImGui::SetCursorPos(
        ImVec2(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 400,
               ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 20));
    ImGui::Text("buffer[2] = %d", buffer[2]);
    ImGui::SetCursorPos(
        ImVec2(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 250,
               ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 20));
    ImGui::Text("data[2] = %d", data[2]);
    */

    // botão de save
    ImGui::Spacing();
    ImGui::NewLine();
    ImVec2 button_size = ImVec2(50, 30);
    ImVec2 available = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x - button_size.x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y - button_size.y);
    if (ImGui::Button("Save", button_size)) {
      memcpy(data, buffer, data_size);
    }

    // botão de load
    button_size = ImVec2(50, 30);
    available = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x - button_size.x -
                         70);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y - button_size.y);
    if (ImGui::Button("Load", button_size)) {
      FILE *file = fopen("entrada.txt",
                         "r");  // arquivo por enquanto está na pasta build
      if (file == NULL) {
        perror("Erro ao abrir o arquivo");
      }
      int16_t value;
      int i = 0;
      while (fscanf(file, "%hd", &value) == 1) {
        buffer[i] = (int16_t)value;
        i++;
      }
      fclose(file);
    }

    ImGui::End();
  }

  // Renderização começa aqui

  // ImGui::ShowDemoWindow();

  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);

  glViewport(0, 0, display_w, display_h);
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

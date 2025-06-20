#include "ui.hpp"﻿
﻿﻿
﻿// INSERIR OS WIDGETS DE VOCÊS AQUI!!!﻿
﻿void RenderMainWindow(GLFWwindow *window, VMState &vm_state) {﻿
﻿    glfwPollEvents();﻿
﻿﻿
﻿    ImGui_ImplOpenGL3_NewFrame();﻿
﻿    ImGui_ImplGlfw_NewFrame();﻿
﻿﻿
﻿        ImGui::NewFrame();﻿
﻿        ﻿
﻿﻿
﻿    // Renderização começa aqui﻿
﻿    static bool show_control_window = true;﻿
﻿    RenderMemoryEditor();﻿
﻿﻿
﻿    RenderMemoryEditor(vm_state);﻿
﻿    RenderControlsWindow(show_control_window, vm_state);﻿
﻿    // ImGui::ShowDemoWindow();﻿
﻿﻿
﻿    ImGui::Render();﻿
﻿    int display_w, display_h;﻿
﻿    glfwGetFramebufferSize(window, &display_w, &display_h);﻿
﻿﻿
﻿    glViewport(0, 0, display_w, display_h);﻿
﻿    glClearColor(0.0f, 0.0f, 0.0f, 1.00f);﻿
﻿    glClear(GL_COLOR_BUFFER_BIT);﻿
﻿﻿
﻿    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());﻿
﻿    glfwSwapBuffers(window);﻿
﻿}﻿
﻿﻿
﻿GLFWwindow *MainWindowSetup(const int width, const int height,﻿
﻿                            const char *title) {﻿
﻿    glfwSetErrorCallback(GLFWErrorCallback);﻿
﻿    if (!glfwInit()) {﻿
﻿        perror("GLFW Error callback failed.");﻿
﻿        exit(EXIT_FAILURE);﻿
﻿    }﻿
﻿﻿
﻿    GLFWwindow *window =﻿
﻿        glfwCreateWindow(width, height, title, nullptr, nullptr);﻿
﻿    if (window == nullptr) {﻿
﻿        perror("GLFW Window Setup failed.");﻿
﻿        exit(EXIT_FAILURE);﻿
﻿    }﻿
﻿﻿
﻿    glfwMakeContextCurrent(window);﻿
﻿    glfwSwapInterval(1);  // vsync﻿
﻿﻿
﻿    return window;﻿
﻿}﻿
﻿﻿
﻿void GLFWErrorCallback(int error, const char *description) {﻿
﻿    fprintf(stderr, "GLFW Error %d: %s\n", error, description);﻿
﻿}﻿
﻿﻿
﻿void IMGUIsetup(GLFWwindow *window) {﻿
﻿    const char *glsl_version = "#version 130";﻿
﻿    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);﻿
﻿    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);﻿
﻿﻿
﻿    IMGUI_CHECKVERSION();﻿
﻿    ImGui::CreateContext();﻿
﻿    ImGuiIO &io = ImGui::GetIO();﻿
﻿    (void)io;﻿
﻿    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;﻿
﻿﻿
﻿    ImGui::StyleColorsDark();﻿
﻿﻿
﻿    ImGui_ImplGlfw_InitForOpenGL(window, true);﻿
﻿    ImGui_ImplOpenGL3_Init(glsl_version);﻿
﻿}﻿
﻿﻿
﻿void WindowCleanup(GLFWwindow *window) {﻿
﻿    ImGui_ImplOpenGL3_Shutdown();﻿
﻿    ImGui_ImplGlfw_Shutdown();﻿
﻿    ImGui::DestroyContext();﻿
﻿﻿
﻿    glfwDestroyWindow(window);﻿
﻿    glfwTerminate();﻿
﻿}﻿
﻿
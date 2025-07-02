#pragma once
#define USE_STD_FILESYSTEM

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <locale>
#include <string>
#include <vector>

#include "external/ImGuiFileDialog.h"
#include "external/ImGuiFileDialogConfig.h"
#include "external/codicon_cpp_header.h"
#include "external/imgui_memory_editor.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "vm.hpp"

typedef struct highlightdata {
  MemoryEditor *memEdit;
  int16_t pc;
} HighlightData;

void GLFWErrorCallback(int error, const char *description);
GLFWwindow *MainWindowSetup(const int width, const int height,
                            const char *title);
void IMGUIsetup(GLFWwindow *window);
void RenderMemoryEditor(VMState &vm_state, bool &window);
void RenderMainWindow(GLFWwindow *window, VMState &vm);
void RenderControlsWindow(bool &window, VMState &vm);
void RenderVMState(VMState &vm, bool &window);
void RenderConsoleWindow(VMState &vm, bool &window);
void WindowCleanup(GLFWwindow *window);
void SkipToEnd(VMState &vm);
void SetupCtanistStyle();

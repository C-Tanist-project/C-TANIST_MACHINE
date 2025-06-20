#ifndef H_UI
#define H_UI
#define USE_STD_FILESYSTEM

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <filesystem>
#include <vector>

#include "external/ImGuiFileDialog.h"
#include "external/ImGuiFileDialogConfig.h"
#include "external/imgui_memory_editor.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "vm.hpp"
void GLFWErrorCallback(int error, const char *description);
GLFWwindow *MainWindowSetup(const int width, const int height,
                            const char *title);
void IMGUIsetup(GLFWwindow *window);
void RenderMemoryEditor(VMState *vm_state);
void RenderMainWindow(GLFWwindow *window, VMState *vm_state);
void RenderControlsWindow(bool &window, VMState *vm_state);
void RenderVMState(VMState *vm_state);
void WindowCleanup(GLFWwindow *window);
void SkipToEnd();

#endif  // !H_UI

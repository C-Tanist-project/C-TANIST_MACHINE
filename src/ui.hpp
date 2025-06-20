#ifndef H_UI
#define H_UI
#define __STDC_WANT_LIB_EXT1__ 1

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

void GLFWErrorCallback(int error, const char *description);
GLFWwindow *MainWindowSetup(const int width, const int height,
                            const char *title);
void IMGUIsetup(GLFWwindow *window);
void RenderMemoryEditor();
void RenderMainWindow(GLFWwindow *window);
void WindowCleanup(GLFWwindow *window);

#endif  // !H_UI

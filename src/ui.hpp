#ifndef H_UI
#define H_UI

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "L2DFileDialog.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"

void GLFWErrorCallback(int error, const char *description);
GLFWwindow *MainWindowSetup(const int width, const int height,
                            const char *title);
void IMGUIsetup(GLFWwindow *window);
void RenderMemoryEditor();
void RenderMainWindow(GLFWwindow *window);
void WindowCleanup(GLFWwindow *window);
void RenderControlsWindow(bool &window);
void SkipToEnd();

#endif  // !H_UI

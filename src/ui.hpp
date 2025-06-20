#ifndef H_UI
#define H_UI

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void GLFWErrorCallback(int error, const char *description);
GLFWwindow *MainWindowSetup(const int width, const int height,
                            const char *title);
void IMGUIsetup(GLFWwindow *window);
void RenderMainWindow(GLFWwindow *window, VMState *vmState);
void RenderVMState(VMState *vmState);
void WindowCleanup(GLFWwindow *window);

#endif  // !H_UI

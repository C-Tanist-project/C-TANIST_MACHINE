#include <stdio.h>
#include <stdlib.h>

#include "ui.hpp"

int main() {
    GLFWwindow *window = MainWindowSetup(1280, 720, "Pentacle VM");

    IMGUIsetup(window);

    while (!glfwWindowShouldClose(window)) {
        RenderMainWindow(window);
    }

    WindowCleanup(window);

    return 0;
}

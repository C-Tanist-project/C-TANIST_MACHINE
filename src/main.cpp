
#include <fstream>
#include <iostream>
#include <thread>

#include "ui.hpp"
#include "vm.hpp"

int main(int argc, char *argv[]) {
    Operations::InitializeMap();

    VMState *vm = VMStateSetup();

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file!\n";
        return 1;
    }

    int16_t instructionsMemoryAddress = 32;

    unsigned char buffer[2];

    while (file.read(reinterpret_cast<char *>(buffer), sizeof(buffer))) {
        u_int16_t word = (buffer[0] << 8) | (buffer[1]);
        vm->memory[instructionsMemoryAddress] = word;
        ++instructionsMemoryAddress;
    }

    std::thread engine(VMEngine, vm);
    GLFWwindow *window = MainWindowSetup(1280, 720, "Pentacle VM");

    IMGUIsetup(window);

    for (int i = 31; i < 50; ++i) {
        std::cout << vm->memory[i] << " ";
    }
    while (!glfwWindowShouldClose(window)) {
        std::unique_lock lock(vm->mutex);
        RenderMainWindow(window, vm);
        lock.unlock();
    }
    engine.join();
    WindowCleanup(window);

    return 0;
}

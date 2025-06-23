#include "src/ui.hpp"

void RenderControlsWindow(bool &window, VMState &vm) {
  ImVec2 initialWindowSize = ImVec2(350, 200);
  ImGui::SetNextWindowSize(initialWindowSize, ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Controls", &window, ImGuiWindowFlags_None)) {
    ImGui::AlignTextToFramePadding();
    ImVec2 buttonSize = ImVec2(40, 20);

    ImGui::Text("Controls");
    ImGui::Separator();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();

    if (ImGui::Button(">=", buttonSize)) {
      vm.isHalted = !vm.isHalted;
      std::cout << "Is halted: " << (vm.isHalted ? "true" : "false") << "\n";
    }
    ImGui::SameLine();
    if (ImGui::Button(">>", buttonSize)) {
      SkipToEnd(vm);
    }
    ImGui::SameLine();

    float windowWidth = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + windowWidth -
                         5 * buttonSize.x);

    ImGui::AlignTextToFramePadding();
    if (ImGui::Button("Step", buttonSize)) {
      vm.sigStep = !vm.sigStep;
      std::cout << "Stepping...\n";
    }

    ImGui::NewLine();
    ImGui::Text("Speed");

    ImGui::PushItemWidth(windowWidth - 20);
    static float speed = 0.0f;
    if (ImGui::SliderFloat("##SpeedSlider", &speed, 0.0f, 10.0f, "%.4f",
                           ImGuiSliderFlags_Logarithmic)) {
      std::cout << "Speed changed to: " << speed << "\n";
    }
    ImGui::PopItemWidth();
  }

  ImGui::End();
}

void SkipToEnd(VMState &vm) {
  if (vm.isHalted) {
    std::cout << "Skipping to the end of the program...\n";
    vm.sigFinish = true;
  }
}

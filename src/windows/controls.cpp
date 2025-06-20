#include "src/ui.hpp"

void RenderControlsWindow(bool &window, VMState *vm_state) {
  ImVec2 initial_window_size = ImVec2(350, 200);
  ImGui::SetNextWindowSize(initial_window_size, ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Controls", &window, ImGuiWindowFlags_None)) {
    ImGui::AlignTextToFramePadding();
    ImVec2 button_size = ImVec2(40, 20);

    ImGui::Text("Controls");
    ImGui::Separator();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();

    if (ImGui::Button(">=", button_size)) {
      vm_state->isHalted = !vm_state->isHalted;
      std::cout << "Is halted: " << (vm_state->isHalted ? "true" : "false")
                << "\n";
    }
    ImGui::SameLine();
    if (ImGui::Button(">>", button_size)) {
      SkipToEnd();
    }
    ImGui::SameLine();

    // Use current window width instead of fixed one
    float window_width = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + window_width -
                         5 * button_size.x);

    ImGui::AlignTextToFramePadding();
    if (ImGui::Button("Step", button_size)) {
      vm_state->sigStep = !vm_state->sigStep;
      std::cout << "Stepping...\n";
    }

    ImGui::NewLine();
    ImGui::Text("Speed");

    ImGui::PushItemWidth(window_width - 20);
    static float speed = 0.0f;
    if (ImGui::SliderFloat("##SpeedSlider", &speed, 0.0f, 10.0f, "%.4f",
                           ImGuiSliderFlags_Logarithmic)) {
      std::cout << "Speed changed to: " << speed << "\n";
    }
    ImGui::PopItemWidth();
  }

  ImGui::End();
}

void SkipToEnd() { std::cout << "Skipping to the end of the program...\n"; }

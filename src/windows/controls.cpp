#include "src/ui.hpp"

void RenderControlsWindow(bool& window) {
    ImVec2 window_size = ImVec2(350, 200);
    ImGui::SetNextWindowSize(window_size);

    if (ImGui::Begin("controls", &window, ImGuiWindowFlags_NoTitleBar)) {
        static bool IsHalted = false;
        static int stepCounter = 0;
        ImVec2 button_size = ImVec2(40, 20);

        ImGui::Text("Controls");
        ImGui::Separator();
        ImGui::NewLine();
        ImGui::NewLine();
        ImGui::NewLine();

        if (ImGui::Button(">=", button_size)) {
            IsHalted = !IsHalted;
            std::cout << "Is halted:" << (IsHalted ? "true" : "false") << "\n";
        }
        ImGui::SameLine();
        if (ImGui::Button(">>", button_size)) {
            SkipToEnd();
        }
        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + window_size.x -
                             5 * button_size.x);

        ImGui::AlignTextToFramePadding();
        if (ImGui::Button("Step", button_size)) {
            stepCounter++;
            std::cout << "Step counter: " << stepCounter << "\n";
        }

        ImGui::NewLine();
        ImGui::Text("Speed");

        ImGui::PushItemWidth(window_size.x - 20);
        static float speed = 0.0f;
        if (ImGui::SliderFloat("##SpeedSlider", &speed, -0.0f, 10.0f, "%.4f",
                               ImGuiSliderFlags_Logarithmic)) {
            std::cout << "Speed changed to: " << speed << "\n";
        }
        ImGui::PopItemWidth();
    }

    ImGui::End();
}

void SkipToEnd() { std::cout << "Skipping to the end of the program...\n"; }

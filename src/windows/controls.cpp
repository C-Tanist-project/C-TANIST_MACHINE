#include "src/ui.hpp"

// O novo modelo de comunicação UI->VM se livra daquele espaguete de atomics.
// Agora é baseado em variáveis de condição. Estou aproveitando a chamada para o
// notify da CV para passar a operação solicitada. Essas operações são
// empilhadas, então dá pra chamar várias delas em sequência.
//
// As chamadas possíveis, por enquanto, são:
//
// STOP: para a máquina
// RUN: inicia a máquina no modo interativo
// FINISH: inicia a máquina no modo não interativo (que ignora o delay de speed)
// STEP: passa para o próximo passo
// CLOSE: fecha a thread da máquina.
//
// Outras chamadas serão definidas em "vm.hpp", no enum "VMControls"
//
// CODICONS é a coleção de ícones do VScode em formato de fonte. Os glifos da
// fonte são chamados via chamada de macro, no formato:
//
// "ICON_CI_{NOME_DO_ICONE}"
//
// Abaixo tem alguns exemplos. Nesse link tem todos os ícones possível com os
// nomes:
//
// https://microsoft.github.io/vscode-codicons/dist/codicon.html

void RenderControlsWindow(bool &window, VMState &vm) {
  ImVec2 initialWindowSize = ImVec2(350, 200);
  ImGui::SetNextWindowSize(initialWindowSize, ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Controls", &window, ImGuiWindowFlags_None)) {
    ImGui::AlignTextToFramePadding();
    ImVec2 buttonSize = ImVec2(30, 30);

    ImGui::Text("Controls");
    ImGui::Separator();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();

    if (ImGui::Button(ICON_CI_DEBUG_STOP, buttonSize)) {
      VMEngine::NotifyCommand(STOP);
      std::cout << "Signal to stop sent...";
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_CI_RUN_ALL, buttonSize)) {
      SkipToEnd(vm);
    }
    ImGui::SameLine();

    float windowWidth = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + windowWidth -
                         5 * buttonSize.x);

    ImGui::AlignTextToFramePadding();
    if (ImGui::Button(ICON_CI_DEBUG_CONTINUE_SMALL, buttonSize)) {
      VMEngine::NotifyCommand(STEP);
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
    VMEngine::NotifyCommand(FINISH);
  } else {
    std::cout << "VM is already running\n";
  }
}

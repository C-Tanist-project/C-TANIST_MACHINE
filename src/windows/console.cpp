#include "src/ui.hpp"

void RenderConsoleWindow(VMState& vm) {
  static int inputValue = 0;
  static bool shouldFocusInput = false;
  static std::vector<std::string> consoleHistory;

  // processa mensagens da VM
  if (ImGui::Begin("Console", NULL, ImGuiWindowFlags_NoCollapse)) {
    {
      std::lock_guard<std::mutex> lock(vm.consoleMutex);
      while (!vm.consoleMessages.empty()) {
        consoleHistory.push_back(vm.consoleMessages.front());
        vm.consoleMessages.pop();
      }
    }

    // output
    ImGui::BeginChild("ConsoleOutput", ImVec2(0, -30), true,
                      ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Text("Console Output:");
    ImGui::Separator();

    // mostra as mensagens do histórico
    for (const auto& message : consoleHistory) {
      ImGui::TextWrapped("%s", message.c_str());
    }

    // mostra texto se experando por input
    if (vm.waitingForInput) {
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                         "Waiting for input...");
    }

    // auto-scroll para o final
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::Separator();

    bool inputEnabled = vm.waitingForInput;

    if (!inputEnabled) {
      ImGui::BeginDisabled();
    }

    // foca no input quando a VM está esperando
    if (vm.waitingForInput && !shouldFocusInput) {
      shouldFocusInput = true;
    }
    if (shouldFocusInput) {
      ImGui::SetKeyboardFocusHere();
      shouldFocusInput = false;
    }

    ImGui::SetNextItemWidth(-70);
    ImGui::InputInt("##InputValue", &inputValue, 0, 0);
    bool enterPressed =
        ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter);
    ImGui::SameLine();

    bool buttonPressed = ImGui::Button("Send");
    if (!inputEnabled) {
      ImGui::EndDisabled();
    }

    // processa o input quando Enter é pressionado ou botão é clicado
    if (inputEnabled && (enterPressed || buttonPressed)) {
      // fornece o input para a VM (por enquanto) (acho que não era pra fazer
      // isso direto mas)
      vm.inputValue = (int16_t)inputValue;
      vm.waitingForInput = false;
      // adiciona mensagem ao console mostrando o input (teste)
      {
        std::lock_guard<std::mutex> lock(vm.consoleMutex);
        vm.consoleMessages.push("Input: " + std::to_string(inputValue));
      }
      inputValue = 0;
    }

    ImGui::End();
  }
}

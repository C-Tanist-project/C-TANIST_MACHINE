#include "src/ui.hpp"

void RenderConsoleWindow(VMState& vm, bool& window) {
  static int inputValue = 0;
  static bool shouldFocusInput = false;
  static std::vector<std::string> consoleHistory;

  // processa mensagens da VM (fila)
  if (window) {
    ImVec2 fixedSize(400, 200);
    ImGui::SetNextWindowSize(fixedSize, ImGuiCond_Always);
    if (ImGui::Begin("Console", &window,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
      {
        std::lock_guard<std::mutex> lock(vm.consoleMutex);
        while (!vm.consoleMessages.empty()) {
          consoleHistory.push_back(vm.consoleMessages.front());
          vm.consoleMessages.pop();
        }
      }

      ImGui::BeginChild("ConsoleOutput", ImVec2(0, -30), true,
                        ImGuiWindowFlags_HorizontalScrollbar);

      // mostra as mensagens do histórico
      for (const auto& message : consoleHistory) {
        ImGui::TextWrapped("%s", message.c_str());
      }

      // mostra texto quando está esperando por input
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

      ImGui::SetNextItemWidth(-70);
      ImGui::InputInt("##InputValue", &inputValue, 0, 0);
      bool enterPressed =
          ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter);
      ImGui::SameLine();

      bool buttonPressed = ImGui::Button("Send");
      if (!inputEnabled) {
        ImGui::EndDisabled();
      }

      // processa o input quando enter é pressionado ou botão é clicado
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
        shouldFocusInput = false;
      }

      // foca o input se a VM está esperando por input
      if (vm.waitingForInput && !shouldFocusInput) {
        shouldFocusInput = true;
      }
      if (shouldFocusInput) {
        ImGui::SetKeyboardFocusHere();
        shouldFocusInput = false;
      }

      ImGui::End();
    }
  }
}

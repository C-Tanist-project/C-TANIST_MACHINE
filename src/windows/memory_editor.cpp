#include <string>

#include "src/ui.hpp"

void RenderMemoryEditor(VMState &vm) {
  static MemoryEditor memEdit;
  // memEdit.PreviewDataType = ImGuiDataType_S16;
  static std::filesystem::path currentPath;
  static bool openDialog = false;

  // Buffer: Vetor sincronizado com as alterações do usuário na interface
  static int16_t buffer[500];
  size_t dataSize = sizeof(vm.memory);
  size_t bufferSize = sizeof(buffer);

  static bool initialized = false;

  if (!initialized) {
    memcpy(buffer, vm.memory, dataSize);
    initialized = true;
  }

  memEdit.OptShowOptions = false;
  memEdit.OptShowDataPreview = true;
  memEdit.OptMidColsCount = 0;
  memEdit.Cols = 8;
  memEdit.OptFooterExtraHeight = 70.0;

  if (memEdit.Open) {
    {
      std::lock_guard<std::mutex> lock(vm.mutex);
      while (!vm.updatedMemoryAddresses.empty()) {
        int16_t updatedAddress = vm.updatedMemoryAddresses.top();
        vm.updatedMemoryAddresses.pop();
        buffer[updatedAddress] = vm.memory[updatedAddress];
      }
    }
    // memEdit.DrawWindow("Mem Edit", buffer, bufferSize);
    ImGui::Begin("Memory Editor", &memEdit.Open);
    memEdit.DrawContents(buffer, bufferSize);

    ImGui::Separator();

    // Para testar se buffer e data estão funcionando como previsto:
    /*
    ImGui::SetCursorPos(
        ImVec2(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
    400, ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 20));
    ImGui::Text("buffer[2] = %d", buffer[2]);
    ImGui::SetCursorPos(
        ImVec2(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
    250, ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 20));
    ImGui::Text("data[2] = %d", vm.memory[2]);
    */

    // Botão SAVE
    // Salva no vetor da VM as modifiações feitas no buffer
    ImGui::BeginChild("##LoadnSave");

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Arquivo selecionado:");
    ImGui::SameLine();
    ImGui::Text("%s", currentPath.filename().c_str());

    ImVec2 buttonSize = ImVec2(50, 30);
    ImVec2 available = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x - buttonSize.x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y - buttonSize.y);
    if (ImGui::Button("Save", buttonSize)) {
      memcpy(&vm.memory, buffer, dataSize);
    }

    // Botão LOAD
    // Permite escolher um arquivo e carregar os dados em buffer

    available = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x - buttonSize.x -
                         70);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y - buttonSize.y);
    if (ImGui::Button("Load", buttonSize)) {
      openDialog = true;
    }

    ImGui::EndChild();
    ImGui::End();

    if (openDialog) {
      IGFD::FileDialogConfig config;
      config.path = ".";
      ImGuiFileDialog::Instance()->OpenDialog(
          "TxtMemoryDialog", "Escolha o arquivo de memória (.txt) (.bin)",
          "Text and Binary{.txt,.bin}", config);
      ImVec2 initialWindowSize = ImVec2(700, 500);
      ImGui::SetNextWindowSize(initialWindowSize, ImGuiCond_FirstUseEver);
    }

    if (ImGuiFileDialog::Instance()->Display("TxtMemoryDialog")) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        std::string filePathName;
        filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
        currentPath.assign(filePathName);

        if (currentPath.extension().string() == ".txt") {
          std::ifstream file(currentPath, std::ios::in);
          int16_t value;
          int i = 0;

          while (file >> value && i < 500) {
            buffer[i] = value;
            i++;
          }
          file.close();
        }
      }

      if (currentPath.extension().string() == ".bin") {
        std::ifstream file(currentPath, std::ios::in | std::ios::binary);
        int16_t value;
        int i = 0;

        while (file.read(reinterpret_cast<char *>(&value), sizeof(int16_t)) &&
               i < 500) {
          buffer[i] = value;
          i++;
        }
        file.close();
      }

      ImGuiFileDialog::Instance()->Close();
      openDialog = false;
    }
  }
}

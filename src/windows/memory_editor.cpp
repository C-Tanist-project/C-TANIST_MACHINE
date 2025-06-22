#include <string>

#include "src/ui.hpp"

void RenderMemoryEditor(VMState *vm) {
  static MemoryEditor memEdit;
  memEdit.PreviewDataType = ImGuiDataType_U16;

  // Buffer: Vetor sincronizado com as alterações do usuário na interface
  static uint16_t buffer[500];
  size_t dataSize = sizeof(vm->memory);
  size_t bufferSize = sizeof(buffer);

  static bool initialized = false;
  if (!initialized) {
    memcpy(buffer, vm->memory, dataSize);
    initialized = true;
  }

  memEdit.OptShowOptions = false;
  memEdit.OptShowDataPreview = true;

  if (memEdit.Open) {
    ImGui::Begin("Memory Editor", &memEdit.Open);
    memEdit.DrawContents(buffer, bufferSize);

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
    ImGui::Spacing();
    ImGui::NewLine();
    ImVec2 buttonSize = ImVec2(50, 30);
    ImVec2 available = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x - buttonSize.x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y - buttonSize.y);
    if (ImGui::Button("Save", buttonSize)) {
      memcpy(&vm->memory, buffer, dataSize);
    }

    // Botão LOAD
    // Permite escolher um arquivo e carregar os dados em buffer
    buttonSize = ImVec2(50, 30);
    available = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x - buttonSize.x -
                         70);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y - buttonSize.y);
    static bool openDialog = false;

    if (ImGui::Button("Load", buttonSize)) {
      openDialog = true;
    }

    if (openDialog) {
      IGFD::FileDialogConfig config;
      config.path = ".";
      ImGuiFileDialog::Instance()->OpenDialog(
          "TxtMemoryDialog", "Escolha o arquivo de memória (.txt)", ".txt",
          config);
      ImVec2 initialWindowSize = ImVec2(700, 500);
      ImGui::SetNextWindowSize(initialWindowSize, ImGuiCond_FirstUseEver);
    }
    // Depois que o caminho for preenchido
    static std::string currentPath;
    if (ImGuiFileDialog::Instance()->Display("TxtMemoryDialog")) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        std::string filePathName;
        std::string filePath;
        filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
        filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

        FILE *f = fopen(filePathName.c_str(), "r");
        if (f) {
          int16_t value;
          int i = 0;
          while (fscanf(f, "%hd", &value) == 1) {
            buffer[i] = (int16_t)value;
            i++;
          }
          fclose(f);
        } else {
          std::cout << "Erro ao abrir o arquivo: " << currentPath << "\n";
          perror("Erro ao abrir o arquivo.");
        }
        currentPath = filePathName;
      }
      ImGuiFileDialog::Instance()->Close();
      openDialog = false;
    }
    ImGui::Text("Arquivo selecionado:");
    ImGui::Text("%s", currentPath.c_str());
    ImGui::End();
  }
}

#include "src/ui.hpp"
#include <bitset>

void RewriteBuffer(const std::filesystem::path currentPath,
                   const bool shouldWipeBuffer, int16_t *buffer,
                   size_t bufferSize) {

  if (shouldWipeBuffer) {
    memset(buffer, 0, bufferSize);
  }

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
}

bool CustomHighlights(const ImU8 *mem, size_t offset, void *userData) {
  MemoryEditor *data = (MemoryEditor *)userData;

  size_t current = data->DataPreviewAddr;
  size_t pairStart;
  size_t pairEnd;
  if (data->DataPreviewAddr % 2) {
    pairStart = current - 1;
    pairEnd = current;
  } else {
    pairStart = current;
    pairEnd = current + 1;
  }

  return (offset >= pairStart && offset <= pairEnd);
}

void RenderMemoryEditor(VMState &vm) {
  static MemoryEditor memEdit;
  // memEdit.PreviewDataType = ImGuiDataType_S16;
  static std::filesystem::path currentPath;
  static bool openDialog = false;
  static bool openPopup = false;
  static size_t currentAddress;

  // Buffer: Vetor sincronizado com as alterações do usuário na interface
  static int16_t buffer[500];
  size_t dataSize = 500 * 2;
  size_t bufferSize = sizeof(buffer);

  static bool initialized = false;

  if (!initialized) {
    memcpy(buffer, vm.memory, dataSize);
    initialized = true;
  }

  memEdit.OptShowOptions = false;
  memEdit.OptShowDataPreview = false;
  memEdit.OptMidColsCount = 2;
  memEdit.Cols = 8;
  memEdit.OptAddrDigitsCount = 4;
  memEdit.OptFooterExtraHeight = 120.0;
  memEdit.HighlightFn = CustomHighlights;
  memEdit.UserData = &memEdit;

  if (memEdit.Open) {
    {
      std::lock_guard<std::mutex> lock(vm.mutex);
      while (!vm.updatedMemoryAddresses.empty()) {
        int16_t updatedAddress = vm.updatedMemoryAddresses.top();
        vm.updatedMemoryAddresses.pop();
        buffer[updatedAddress] = vm.memory[updatedAddress];
      }
    }
    ImGui::Begin("Memory Editor", &memEdit.Open);
    memEdit.DrawContents(buffer, bufferSize, 0x0000);

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
    if (memEdit.DataEditingTakeFocus) {
      currentAddress = memEdit.DataEditingAddr / 2;
    }

    ImGui::AlignTextToFramePadding();

    int16_t byteCast = static_cast<int16_t>(buffer[currentAddress]);

    ImGui::Text("Endereço: %zu", currentAddress);
    ImGui::Text("Valor:\n%d (D)\t0x%04X (H)\t%s (B)", byteCast,
                static_cast<uint16_t>(byteCast),
                std::bitset<16>(byteCast).to_string().c_str());

    ImGui::Separator();

    std::string utf8Filename = currentPath.filename().u8string();

    ImGui::Text("Arquivo selecionado:");
    ImGui::SameLine();
    ImGui::Text("%s", utf8Filename.c_str());

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

    if (openPopup) {
      ImGui::OpenPopup("ClearBuffer");
      openPopup = false;
    }

    if (ImGui::BeginPopupModal("ClearBuffer", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      if (ImGui::Button("Wipe")) {
        RewriteBuffer(currentPath, true, buffer, 500);
        ImGui::CloseCurrentPopup();
      }

      if (ImGui::Button("Overwrite")) {
        RewriteBuffer(currentPath, false, buffer, 500);
        ImGui::CloseCurrentPopup();
      }

      if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }
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

      ImGuiFileDialog::Instance()->Close();
      openDialog = false;
      openPopup = true;
    }
  }
}

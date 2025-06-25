#include "src/ui.hpp"
#include <bitset>

// REWRITEBUFFER: Função que trata a sobrescrição do buffer de entrada ao
// carregar arquivos no inspetor.
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

// CUSTOMHIGHLIGHTS: trunca o highlight no byte selecionado com a posição de
// memória associada a ele.
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

  // Opções do editor de memória
  memEdit.OptShowOptions =
      false; // Desabilita o menu de opções (não ajuda muito)
  memEdit.OptShowDataPreview = false; // Desabilita o menu de preview (versão
                                      // customizada disso já foi implementada)
  memEdit.OptMidColsCount = 2;    // Quebra os 4 bytes em conjuntos de 16 bits
  memEdit.Cols = 8;               // 4 bytes
  memEdit.OptAddrDigitsCount = 4; // Mostra sempre 4 bytes no endereçamento
  memEdit.OptFooterExtraHeight =
      120.0; // Espaço extra no fim do módulo para widgets extra
  memEdit.HighlightFn = CustomHighlights; // ditto
  memEdit.UserData =
      &memEdit; // Utiizado em CustomHighlights: é o estado do próprio editor

  if (memEdit.Open) {
    // ATENÇÃO AQUI:
    // Pra melhorar o desempenho (não ter um memcpy a cada frame) eu implementei
    // um sistema de pilha de mudanças na memória. Cada vez que a VM escrever na
    // memória, a alteração é empilhada; a cada frame, as alterações são
    // desempilhadas e atualizadas no buffer local.
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

    ImGui::BeginChild("##LoadnSave");
    // Converte o endereço destacado no editor de memória (que é endereçado à
    // byte) para o espaço de endereçamento da VM (16 b)
    if (memEdit.DataEditingTakeFocus) {
      currentAddress = memEdit.DataEditingAddr / 2;
    }

    ImGui::AlignTextToFramePadding();

    // Essse cast faz as conversões bit-a-bit funcionarem da mesma forma nas
    // duas plataformas. A conversão para binário não estava funcionando
    // corretamente com o int16_t direto no Windows. Esse modelo permite
    // traduzir o valor para binário através de um bitset (que depois é
    // convertido em string).
    int16_t byteCast = static_cast<int16_t>(buffer[currentAddress]);

    ImGui::Text("Endereço: %zu", currentAddress);
    ImGui::Text("Valor:\n%d (D)\t0x%04X (H)\t%s (B)", byteCast,
                static_cast<uint16_t>(byteCast),
                std::bitset<16>(byteCast).to_string().c_str());

    ImGui::Separator();

    // o método u8string especifica que a string do caminho é pra ser formatada
    // sempre em utf8. No windows, por padrão, essas strings se tornam utf-16, o
    // que quebra ao converter para a string padrão C (que é
    // o padrão que o IMGUI usa no Text())
    std::string utf8Filename = currentPath.filename().u8string();

    ImGui::Text("Arquivo selecionado:");
    ImGui::SameLine();
    ImGui::Text("%s", utf8Filename.c_str());

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

    // Eu coloquei tudo que é impresso em um child pra separar melhor em código
    // o que é o inspetor de memória, seletor de arquivos e popup de método de
    // escrita. Mais sobre essas adições abaixo.
    ImGui::EndChild();

    // Esse popup serve para escolher o método de escrita depois de selecionar
    // um arquivo: Overwrite escreve o conteúdo do arquivo por cima do que já
    // está no buffer; Wipe limpa o buffer antes de escrever o arquivo e Cancel
    // cancela (duh) a operação de escrita.
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

  // Janela de seleção de arquivos
  if (openDialog) {
    IGFD::FileDialogConfig config;
    // Abre a janela na pasta atual (em que o programa foi lançado).
    config.path = ".";
    // O membro 2 dessa chamada de abertura é a mudança mais importante: é um
    // regex que aceita arquivos terminados em .txt e .bin (aparece na tela como
    // "Text and Binary")
    ImGuiFileDialog::Instance()->OpenDialog(
        "TxtMemoryDialog", "Escolha o arquivo de memória (.txt) (.bin)",
        "Text and Binary{.txt,.bin}", config);
    // Isso aqui corrige a janela abrindo minúscula na primeira vez.
    ImVec2 initialWindowSize = ImVec2(700, 500);
    ImGui::SetNextWindowSize(initialWindowSize, ImGuiCond_FirstUseEver);
  }

  // Esse é o bloco que renderiza a janela de seleção de arquivos quando
  // Instance()->OpenDialog é chamado.
  if (ImGuiFileDialog::Instance()->Display("TxtMemoryDialog")) {
    // Abaixo disso é o que acontece depois de pressionar OK na janela: copia o
    // endereço do arquivo selecionado para a variável de armazenamento e abre o
    // popup de modo de escrita.
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

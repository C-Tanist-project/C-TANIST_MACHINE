#include <string>

#include "src/ui.hpp"

void RenderMemoryEditor(VMState *vm_state) {
    static MemoryEditor mem_edit;
    mem_edit.PreviewDataType = ImGuiDataType_U16;

    // Buffer: Vetor sincronizado com as alterações do usuário na interface
    static uint16_t buffer[500];
    size_t data_size = sizeof(vm_state->memory);
    size_t buffer_size = sizeof(buffer);

    static bool initialized = false;
    if (!initialized) {
        memcpy(buffer, vm_state->memory, data_size);
        initialized = true;
    }

    mem_edit.OptShowOptions = false;
    mem_edit.OptShowDataPreview = true;

    if (mem_edit.Open) {
        ImGui::Begin("Memory Editor", &mem_edit.Open);
        mem_edit.DrawContents(buffer, buffer_size);

        // Para testar se buffer e data estão funcionando como previsto:
        /*
        ImGui::SetCursorPos(
            ImVec2(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
        400, ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 20));
        ImGui::Text("buffer[2] = %d", buffer[2]);
        ImGui::SetCursorPos(
            ImVec2(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
        250, ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 20));
        ImGui::Text("data[2] = %d", vm_state.memory[2]);
        */

        // Botão SAVE
        // Salva no vetor da VM as modifiações feitas no buffer
        ImGui::Spacing();
        ImGui::NewLine();
        ImVec2 button_size = ImVec2(50, 30);
        ImVec2 available = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x -
                             button_size.x);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y -
                             button_size.y);
        if (ImGui::Button("Save", button_size)) {
            memcpy(&vm_state->memory, buffer, data_size);
        }

        // Botão LOAD
        // Permite escolher um arquivo e carregar os dados em buffer
        button_size = ImVec2(50, 30);
        available = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x -
                             button_size.x - 70);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y -
                             button_size.y);
        static bool openDialog = false;

        if (ImGui::Button("Load", button_size)) {
            openDialog = true;
        }

        if (openDialog) {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog(
                "TxtMemoryDialog", "Escolha o arquivo de memória (.txt)",
                ".txt", config);
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
                    std::cout << "Erro ao abrir o arquivo: " << currentPath
                              << "\n";
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

#include <iostream>
#include <string>

#include "src/ui.hpp"

void RenderMemoryEditor() {
    static MemoryEditor mem_edit;
    mem_edit.PreviewDataType = ImGuiDataType_U16;

    static uint16_t buffer[500], data[500];
    size_t data_size = sizeof(data);
    size_t buffer_size = sizeof(buffer);

    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < 500; ++i) buffer[i] = (uint16_t)0;
        memcpy(data, buffer, data_size);
        initialized = true;
    }

    mem_edit.OptShowOptions = false;
    mem_edit.OptShowDataPreview = true;

    if (mem_edit.Open) {
        ImGui::Begin("Memory Editor", &mem_edit.Open);
        mem_edit.DrawContents(buffer, buffer_size);

        ImGui::Spacing();
        ImGui::NewLine();
        ImVec2 button_size = ImVec2(50, 30);
        ImVec2 available = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x -
                             button_size.x);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y -
                             button_size.y);
        if (ImGui::Button("Save", button_size)) {
            memcpy(data, buffer, data_size);
        }

        button_size = ImVec2(50, 30);
        available = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available.x -
                             button_size.x - 70);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + available.y -
                             button_size.y);

        static bool openDialog = false;

        // Botão LOAD
        if (ImGui::Button("Load", button_size)) {
            openDialog = true;
        }

        // Janela do diálogo de arquivos
        if (openDialog) {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog(
                "TxtMemoryDialog", "Escolha o arquivo de memória (.txt)",
                ".txt", config);
        }

        static std::string currentPath;

        if (ImGuiFileDialog::Instance()->Display("TxtMemoryDialog")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePathName;
                std::string filePath;
                filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                FILE* f = fopen(filePathName.c_str(), "r");
                if (f) {
                    int16_t value;
                    int i = 0;
                    while (fscanf(f, "%hd", &value) == 1) {
                        buffer[i] = (int16_t)value;
                        i++;
                    }
                    fclose(f);
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

#include "src/ui.hpp"
#include "src/vm.hpp"

// casts para o tipo correto (por nenhum motivo em particular)
void DrawRegisterPair(const char *label1, int16_t *reg1, const char *label2,
                      int16_t *reg2) {
  ImGui::PushID(label1);
  // o maior numero que vai aparecer aqui é ~-32000, não precisa comodar muito
  // mais dígitos que isso
  ImGui::SetNextItemWidth(52.5);

  ImGui::InputScalar(label1, ImGuiDataType_S16, reg1, nullptr, nullptr, "%d",
                     ImGuiInputTextFlags_ReadOnly);

  ImGui::SameLine(0, 24);

  ImGui::SetNextItemWidth(52.5);
  ImGui::InputScalar(label2, ImGuiDataType_S16, reg2, nullptr, nullptr, "%d",
                     ImGuiInputTextFlags_ReadOnly);
  ImGui::PopID();
}

void RenderVMState(VMState &vmState, bool &window) {
  if (window) {
    if (ImGui::Begin("Registradores", &window,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Dummy(ImVec2(0, 5));
      // ImGui::Indent(3 * ImGui::GetStyle().IndentSpacing / 2);

      DrawRegisterPair("PC ", &vmState.pc, "SP ", &vmState.sp);
      ImGui::Dummy(ImVec2(0, 4));
      DrawRegisterPair("ACC", &vmState.acc, "MOP", &vmState.mop);
      ImGui::Dummy(ImVec2(0, 4));
      DrawRegisterPair("R0 ", &vmState.r0, "R1 ", &vmState.r1);
      ImGui::Dummy(ImVec2(0, 4));
      DrawRegisterPair("RE ", &vmState.re, "RI ", &vmState.ri);
      ImGui::Dummy(ImVec2(0, 6));
    }
    ImGui::End();
  }
}

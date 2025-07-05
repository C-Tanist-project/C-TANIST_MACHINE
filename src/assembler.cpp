#include "assembler.hpp"

#include "types.hpp"

Assembler::Assembler(const std::string &asmFilePath,
                     const std::string &objFilePath,
                     const std::string &lstFilePath) {
  this->asmFilePath = asmFilePath;
  this->objFilePath = objFilePath;
  this->lstFilePath = lstFilePath;
  this->locationCounter = 0;
  this->lineCounter = 0;
}

AssemblerExitCode Assembler::Assemble() {
  this->finalExitCode = this->FirstPass();
  if (this->finalExitCode != SUCCESS) return finalExitCode;
  this->finalExitCode = this->SecondPass();
  return finalExitCode;
}

AssemblerExitCode Assembler::FirstPass() {
  AssemblerExitCode exitCode = SUCCESS;
  // faz a primeira passagem

  return exitCode;
}

AssemblerExitCode Assembler::SecondPass() {
  AssemblerExitCode exitCode = SUCCESS;

  // faz a (adivinha?) segunda passagem
  return exitCode;
}

// escreve o arquivo .obj com base no vetor this->objectCode
void Assembler::WriteObjectCodeFile() {
  std::ofstream objFile(this->objFilePath, std::ios::binary);
  if (!objFile) {
    std::cerr << "Erro ao abrir o arquivo de código objeto: "
              << this->objFilePath << std::endl;
    return;
  }

  // STACK_SIZE
  objFile.put(static_cast<int16_t>(ObjSectionType::STACK_SIZE));
  objFile.write(reinterpret_cast<const char *>(&this->stackSize),
                sizeof(int16_t));

  // INTDEF
  objFile.put(static_cast<int16_t>(ObjSectionType::INTDEF));
  int16_t defCount = static_cast<int16_t>(this->intDefTable.size());
  objFile.write(reinterpret_cast<const char *>(&defCount), sizeof(int16_t));

  for (const auto &[label, addr] : this->intDefTable) {
    int16_t labelLen = static_cast<int16_t>(label.size());
    objFile.write(reinterpret_cast<const char *>(&labelLen), sizeof(int16_t));
    objFile.write(label.data(), labelLen);
    objFile.write(reinterpret_cast<const char *>(&addr), sizeof(int16_t));
  }

  // INTUSE
  objFile.put(static_cast<int16_t>(ObjSectionType::INTUSE));
  int16_t useCount = static_cast<int16_t>(this->intUseTable.size());
  objFile.write(reinterpret_cast<const char *>(&useCount), sizeof(int16_t));

  for (const auto &[label, addresses] : this->intUseTable) {
    int16_t labelLen = static_cast<int16_t>(label.size());
    objFile.write(reinterpret_cast<const char *>(&labelLen), sizeof(int16_t));
    objFile.write(label.data(), labelLen);

    int16_t addrCount = static_cast<int16_t>(addresses.size());
    objFile.write(reinterpret_cast<const char *>(&addrCount), sizeof(int16_t));
    for (int16_t addr : addresses) {
      objFile.write(reinterpret_cast<const char *>(&addr), sizeof(int16_t));
    }
  }

  // CODE
  objFile.put(static_cast<int16_t>(ObjSectionType::CODE));
  int16_t codeSize = static_cast<int16_t>(this->objectCode.size());
  objFile.write(reinterpret_cast<const char *>(&codeSize), sizeof(int16_t));
  objFile.write(reinterpret_cast<const char *>(this->objectCode.data()),
                this->objectCode.size() * sizeof(int16_t));

  // END
  objFile.put(static_cast<int16_t>(ObjSectionType::END));

  objFile.close();
}

// escreve o arquivo .lst com this->listingLines e this->listingErrors
void Assembler::WriteListingFile() {}
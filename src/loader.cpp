#include "loader.hpp"

Loader::LoadedModule Loader::ReadHPX() {
  std::ifstream in(this->fileName, std::ios::binary);
  if (!in) throw std::runtime_error("Erro ao abrir " + fileName);

  LoadedModule module;
  module.name = this->fileName;

  char magic[4];
  in.read(magic, sizeof(magic));
  if (std::string(magic, 3) != "HPX")
    throw std::runtime_error("Arquivo não é HPX válido");

  // entryPoint (comeco do programa)
  in.read(reinterpret_cast<char *>(&module.entryPoint),
          sizeof(module.entryPoint));

  // stackSize (pilha)
  in.read(reinterpret_cast<char *>(&module.stackSize),
          sizeof(module.stackSize));

  // code
  int32_t codeSize;
  in.read(reinterpret_cast<char *>(&codeSize), sizeof(codeSize));
  module.code.resize(codeSize);
  in.read(reinterpret_cast<char *>(module.code.data()),
          codeSize * sizeof(int16_t));

  // tabela de realocação
  int32_t relocSize;
  in.read(reinterpret_cast<char *>(&relocSize), sizeof(relocSize));
  for (int i = 0; i < relocSize; ++i) {
    int16_t offset;
    OperandFormat fmt;
    in.read(reinterpret_cast<char *>(&offset), sizeof(offset));
    in.read(reinterpret_cast<char *>(&fmt), sizeof(fmt));
    module.relocationTable[offset] = fmt;
  }

  return module;
}

void Loader::RelocateModule() {
  // realoca cada posição (com base no loadAddress)
  for (auto &[offset, fmt] : this->module.relocationTable) {
    switch (fmt) {
      case OperandFormat::IMMEDIATE:
        continue;
      case OperandFormat::DIRECT:
      case OperandFormat::INDIRECT: {
        if (offset < 0 ||
            offset >= static_cast<int16_t>(this->module.code.size())) {
          throw std::out_of_range(
              "Endereço de relocação fora dos limites do código");
        }
        this->module.code[offset] += this->module.loadAddress;
        continue;
      }
    }
  }
}

void Loader::Pass() {
  ReadHPX();
  RelocateModule();
}

Loader::Loader(const std::string &fileName) { this->fileName = fileName; }
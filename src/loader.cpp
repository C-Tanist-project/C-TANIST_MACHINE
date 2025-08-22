#include "loader.hpp"

#include <stdio.h>
#include <stdlib.h>

// using RelocOffset = int16_t;
// using RelocFormat = int16_t;

void Loader::ReadHPX(const std::filesystem::path &filePath) {
  std::ifstream in(filePath, std::ios::binary);
  if (!in) {
    throw std::runtime_error("Erro: Não foi possível abrir o arquivo " +
                             filePath.string());
  }

  std::cout << "--- Lendo arquivo HPX: " << filePath.string() << " ---\n\n";

  // 1. Ler e verificar o Magic Number
  char magic[4];
  in.read(magic, sizeof(magic));
  std::cout << "MAGIC NUMER LIDO:" << magic << std::endl;
  if (std::string(magic, 3) != "HPX") {
    throw std::runtime_error("Erro: O arquivo não é um formato HPX válido.");
  }
  std::cout << "[Cabeçalho]\n";
  std::cout << "  Magic Number: HPX (Válido)\n";

  // 2. Ler Entry Point e Tamanho da Pilha
  int16_t entryPoint, stackSize;
  in.read(reinterpret_cast<char *>(&entryPoint), sizeof(entryPoint));
  in.read(reinterpret_cast<char *>(&stackSize), sizeof(stackSize));
  std::cout << "  Entry Point: " << entryPoint << "\n";
  std::cout << "  Tamanho da Pilha: " << stackSize << " bytes\n\n";

  this->module.entryPoint = entryPoint;
  this->module.stackSize = stackSize;

  // 3. Ler a seção de código
  int16_t codeSize;
  in.read(reinterpret_cast<char *>(&codeSize), sizeof(codeSize));
  std::vector<int16_t> code(codeSize);
  in.read(reinterpret_cast<char *>(code.data()), codeSize * sizeof(int16_t));

  std::cout << "[Seção de Código] (" << codeSize << " instruções)\n";
  for (int i = 0; i < codeSize; ++i) {
    std::cout << "  " << std::setw(4) << std::setfill('0') << i << ": 0x"
              << std::hex << std::setw(4) << std::setfill('0')
              << static_cast<int16_t>(code[i]) << std::dec << "\n";
    this->module.code.push_back(static_cast<int16_t>(code[i]));
  }
  std::cout << "\n";

  // 4. Ler a tabela de relocação
  int16_t relocSize;
  in.read(reinterpret_cast<char *>(&relocSize), sizeof(int16_t));

  for (int i = 0; i < relocSize; i++) {
    int16_t offset;
    int16_t fmt;
    in.read(reinterpret_cast<char *>(&offset), sizeof(int16_t));
    in.read(reinterpret_cast<char *>(&fmt), sizeof(int16_t));

    if (fmt == 0) {
      // Diretamente relocável → soma base
      module.code[offset] += this->module.stackSize + 4;
    } else {
      // Indireto → soma no valor apontado e também no endereço
      int16_t targetIndex = module.code[offset];
      module.code[targetIndex] += this->module.stackSize + 4;
      module.code[offset] += this->module.stackSize + 4;
    }
  }

  std::cout << "\n--- Fim da Leitura ---\n";
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

void Loader::Pass(std::filesystem::path &filePath) {
  std::filesystem::path file;

  for (const auto &entry :
       std::filesystem::directory_iterator(filePath / "HPX")) {
    if (std::filesystem::is_regular_file(entry.status())) {
      std::cout << entry << std::endl;
      file = entry;
    }
  }
  ReadHPX(file);
  RelocateModule();
}

bool Loader::SetMemory(VMState &vm) {
  std::vector<int16_t> memory = this->module.code;

  vm.memory[2] = this->module.stackSize;
  // vm.memory[1] = this->module.entryPoint;
  vm.memory[this->module.stackSize + 3] =
      this->module.entryPoint + this->module.stackSize + 4;

  vm.memory[1] = this->module.stackSize + 3;

  int j = 0;
  if (memory.size() + 4 + this->module.stackSize > 500) {
    return false;
  } else {
    for (int i = this->module.stackSize + 4;
         i < this->module.code.size() + this->module.stackSize + 4; ++i) {
      vm.memory[i] = this->module.code[j++];
    }
    return true;
  }
}

Loader::Loader() {}

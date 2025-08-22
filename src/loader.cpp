#include "loader.hpp"

using RelocOffset = int16_t;
using RelocFormat = int16_t;

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

  // 3. Ler a seção de código
  int16_t codeSize;
  in.read(reinterpret_cast<char *>(&codeSize), sizeof(codeSize));
  std::vector<int16_t> code(codeSize);
  in.read(reinterpret_cast<char *>(code.data()), codeSize * sizeof(int16_t));

  std::cout << "[Seção de Código] (" << codeSize << " instruções)\n";
  for (int i = 0; i < codeSize; ++i) {
    std::cout << "  " << std::setw(4) << std::setfill('0') << i << ": 0x"
              << std::hex << std::setw(4) << std::setfill('0')
              << static_cast<uint16_t>(code[i]) << std::dec << "\n";
  }
  std::cout << "\n";

  // 4. Ler a tabela de relocação
  int16_t relocSize;
  in.read(reinterpret_cast<char *>(&relocSize), sizeof(relocSize));

  std::cout << "[Tabela de Relocação] (" << relocSize << " entradas)\n";
  for (int32_t i = 0; i < relocSize; ++i) {
    RelocOffset offset;
    RelocFormat fmt;
    in.read(reinterpret_cast<char *>(&offset), sizeof(offset));
    in.read(reinterpret_cast<char *>(&fmt), sizeof(fmt));
    std::cout << "  Offset: " << std::setw(4) << offset << " | Formato: " << fmt
              << "\n";
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

Loader::Loader() {}

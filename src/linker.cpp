#include "linker.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

Linker::Linker() {}

void Linker::GenerateOutput(const std::string &outputName) {
  // criar o .hpx contendo código binário + tabela de relocação global
}
void Linker::SecondPass() {

  // Concatenar os módulos
  // Resolver os símbolos da intUseTable
  // criar tabela de relocação global

};
void Linker::FirstPass(const std::vector<std::string> &objFilePaths) {
  modules.clear();
  globalSymbolTable.clear();
  errors.clear();
  totalStackSize = 0;
  currentLoadAddress = 0;

  for (const auto &filePath : objFilePaths) {
    ReadObjectCodeFile(filePath);
  }

  for (const auto &[name, symbol] : globalSymbolTable) {
    if (symbol.first == UNRESOLVED_ADDRESS) {
      errors.push_back("Símbolo global não resolvido: " + name);
    }
  }

  for (auto &mod : modules) {
    for (auto &intuse : mod.intUseTable) {
      auto &positions = intuse.second;
      for (auto &pos : positions) {
        pos += mod.loadAddress;
      }
    }

    std::unordered_map<int16_t, OperandFormat> newRelocationTable;
    for (const auto &rel : mod.relocationTable) {
      const int16_t locaIndex = rel.first;
      const OperandFormat &format = rel.second;

      switch (format) {
        case IMMEDIATE:
          break;
        case DIRECT:
        case INDIRECT:
          mod.code[locaIndex] += mod.loadAddress;
          break;
        default:
          errors.push_back("Tipo de relocação desconhecido: " +
                           std::to_string(static_cast<int>(format)));
          break;
      }

      const int16_t globalIndex = locaIndex + mod.loadAddress;
      newRelocationTable[globalIndex] = format;
    }
    mod.relocationTable = std::move(newRelocationTable);
  }
}

// escreve o arquivo .obj com base no vetor this->objectCode
void Linker::ReadObjectCodeFile(const std::string &filePath) {
  std::ifstream objFile(filePath, std::ios::binary);
  if (!objFile) {
    std::cerr << "Erro ao abrir o arquivo de código objeto: " << filePath
              << std::endl;
    return;
  }

  Module module;
  module.name = filePath;
  module.loadAddress = currentLoadAddress;

  bool finished = false;
  while (!finished) {
    int16_t rawType;
    if (!objFile.read(reinterpret_cast<char *>(&rawType), sizeof(int16_t))) {
      break;
    }

    ObjSectionType section = static_cast<ObjSectionType>(rawType);

    switch (section) {
      case ObjSectionType::NAME: {
        int16_t nameLen;
        objFile.read(reinterpret_cast<char *>(&nameLen), sizeof(int16_t));

        std::string name(nameLen, '\0');
        objFile.read(&name[0], nameLen);

        module.startName = name;

        break;
      }
      case ObjSectionType::STACK_SIZE: {
        objFile.read(reinterpret_cast<char *>(&module.stackSize),
                     sizeof(int16_t));
        totalStackSize += module.stackSize;
        break;
      }

      case ObjSectionType::INTDEF: {
        int16_t defCount;
        objFile.read(reinterpret_cast<char *>(&defCount), sizeof(int16_t));

        for (int i = 0; i < defCount; ++i) {
          int16_t labelLen;
          objFile.read(reinterpret_cast<char *>(&labelLen), sizeof(int16_t));

          std::string label(labelLen, '\0');
          objFile.read(&label[0], labelLen);

          int16_t address;
          objFile.read(reinterpret_cast<char *>(&address), sizeof(int16_t));

          int16_t relocatedAddr = address + module.loadAddress;

          if (globalSymbolTable.count(label) &&
              globalSymbolTable[label].first != UNRESOLVED_ADDRESS) {
            errors.push_back("Símbolo global já definido: " + label + " [" +
                             filePath + "/" + globalSymbolTable[label].second +
                             "]");
          } else {
            globalSymbolTable[label] = {relocatedAddr, filePath};
            module.intDefTable[label] = relocatedAddr;
          }
        }
        break;
      }

      case ObjSectionType::INTUSE: {
        int16_t useCount;
        objFile.read(reinterpret_cast<char *>(&useCount), sizeof(int16_t));

        for (int i = 0; i < useCount; ++i) {
          int16_t labelLen;
          objFile.read(reinterpret_cast<char *>(&labelLen), sizeof(int16_t));

          std::string label(labelLen, '\0');
          objFile.read(&label[0], labelLen);

          int16_t addrCount;
          objFile.read(reinterpret_cast<char *>(&addrCount), sizeof(int16_t));

          std::vector<int16_t> addresses(addrCount);
          for (int j = 0; j < addrCount; ++j) {
            int16_t addr;
            objFile.read(reinterpret_cast<char *>(&addr), sizeof(int16_t));
            addresses[j] = addr;
          }

          module.intUseTable[label] = std::move(addresses);

          if (!globalSymbolTable.contains(label)) {
            globalSymbolTable[label] = {UNRESOLVED_ADDRESS, filePath};
          }
        }
        break;
      }

      case ObjSectionType::CODE: {
        int16_t codeSize;
        objFile.read(reinterpret_cast<char *>(&codeSize), sizeof(int16_t));
        module.code.resize(codeSize);
        objFile.read(reinterpret_cast<char *>(module.code.data()),
                     codeSize * sizeof(int16_t));
        break;
      }

      case ObjSectionType::RELOCATION: {
        int16_t relocCount;
        objFile.read(reinterpret_cast<char *>(&relocCount), sizeof(int16_t));

        for (int i = 0; i < relocCount; ++i) {
          int16_t address;
          int16_t typeVal;

          objFile.read(reinterpret_cast<char *>(&address), sizeof(int16_t));
          objFile.read(reinterpret_cast<char *>(&typeVal), sizeof(int16_t));

          module.relocationTable[address] = static_cast<OperandFormat>(typeVal);
        }
        break;
      }

      case ObjSectionType::END:
        finished = true;
        break;

      default:
        errors.push_back("Seção desconhecida no arquivo: " + filePath +
                         " (tipo " + std::to_string(static_cast<int>(section)) +
                         ")");
        finished = true;
        break;
    }
  }

  currentLoadAddress += module.code.size();
  modules.push_back(std::move(module));
  objFile.close();
}

void Linker::printModules() {
  for (const auto &mod : modules) {
    std::cout << "Module Name: " << mod.name << "\n";
    std::cout << "Program Name (start): " << mod.startName << "\n";
    std::cout << "Code: ";
    for (const auto &val : mod.code) {
      std::cout << val << " ";
    }
    std::cout << "\n";

    std::cout << "INTDEF Table:\n";
    for (const auto &[symbol, address] : mod.intDefTable) {
      std::cout << "  " << symbol << " => " << address << "\n";
    }

    std::cout << "INTUSE Table:\n";
    for (const auto &[symbol, positions] : mod.intUseTable) {
      std::cout << "  " << symbol << " => [ ";
      for (int pos : positions) {
        std::cout << pos << " ";
      }
      std::cout << "]\n";
    }

    std::cout << "Relocation Table:\n";
    for (const auto &relocIndex : mod.relocationTable) {
      std::cout << "  Relocate address index: " << relocIndex.first << "\n";
    }

    std::cout << "Global Symbol Table:\n";
    for (const auto &[symbol, where] : globalSymbolTable) {
      std::cout << "Símbolo: " << symbol << " " << "Endereço: " << where.first
                << " "
                << "Módulo: " << where.second << std::endl;
    }
    std::cout << "Stack Size: " << mod.stackSize << "\n";
    std::cout << "Start Address: " << mod.startAddress << "\n";
    std::cout << "Load Address: " << mod.loadAddress << "\n";
    std::cout << "\n";
  }
}

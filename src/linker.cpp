#include "linker.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

Linker::Linker() {}

void Linker::Link(const std::vector<std::string> &objFilePaths) {
  FirstPass(objFilePaths);
  SecondPass();
}

void Linker::GenerateOutput(const std::string &outputName) {
  // criar o .hpx contendo código binário + tabela de relocação global
}
void Linker::SecondPass() {
  linkedCode.clear();
  globalRelocationTable.clear();

  for (auto &mod : modules) {
    for (const auto &intuse : mod.intUseTable) {
      const std::string &symbol = intuse.first;
      const auto it = globalSymbolTable.find(symbol);
      if (it == globalSymbolTable.end() ||
          it->second.first == UNRESOLVED_ADDRESS) {
        continue;
      }

      const int16_t &address = it->second.first;

      for (auto &globalPos : intuse.second) {
        int16_t localPos = globalPos - mod.loadAddress;

        if (localPos < 0 || localPos >= static_cast<int16_t>(mod.code.size())) {
          errors.push_back("Endereço fora dos limites do módulo: " + mod.name +
                           " para o símbolo " + symbol);
          continue;
        }

        mod.code[static_cast<size_t>(localPos)] = address;
      }
    }
  }

  linkedCode.assign(static_cast<size_t>(currentLoadAddress), 0);
  for (const auto &mod : modules) {
    const size_t start = static_cast<size_t>(mod.loadAddress);
    if (start + mod.code.size() > linkedCode.size()) {
      errors.push_back("Módulo " + mod.name + " excede o tamanho do código.");
      continue;
    }
    std::copy(mod.code.begin(), mod.code.end(), linkedCode.begin() + start);
  }

  for (const auto &mod : modules) {
    for (const auto &reloc : mod.relocationTable) {
      globalRelocationTable[reloc.first] = reloc.second;
    }
    for (const auto &intuse : mod.intUseTable) {
      for (auto globalPos : intuse.second) {
        const int16_t key = static_cast<int16_t>(globalPos);
        if (!globalRelocationTable.contains(key)) {
          globalRelocationTable[key] = DIRECT;
        }
      }
    }
  }
};

void Linker::FirstPass(const std::vector<std::string> &objFilePaths) {
  modules.clear();
  globalSymbolTable.clear();
  errors.clear();
  globalStackSize = 0;
  currentLoadAddress = 0;

  for (const auto &filePath : objFilePaths) {
    ReadObjectCodeFile(filePath);
  }

  for (auto &mod : modules) {
    for (const auto &[symbol, address] : mod.intDefTable) {
      int16_t relocatedAddr = address + mod.loadAddress;
      if (globalSymbolTable.contains(symbol)) {
        errors.push_back("Símbolo " + symbol + " já definido globalmente.");
      } else {
        globalSymbolTable[symbol] = {relocatedAddr, mod.name};
      }
    }
    for (auto &[symbol, positions] : mod.intUseTable) {
      if (!globalSymbolTable.contains(symbol)) {
        errors.push_back("Símbolo " + symbol + " não definido globalmente.");
      } else {
        for (auto &pos : positions) {
          pos += mod.loadAddress;
        }
      }
    }

    std::unordered_map<int16_t, OperandFormat> newRelocationTable;
    newRelocationTable.reserve(mod.relocationTable.size());

    for (const auto &reloc : mod.relocationTable) {
      const int16_t localIndex = reloc.first;
      const OperandFormat format = reloc.second;

      switch (format) {
        case IMMEDIATE:
          break;
        case DIRECT:
        case INDIRECT:
          mod.code[localIndex] += mod.loadAddress;
          break;
        default:
          errors.push_back("Tipo de relocação desconhecido: " +
                           std::to_string(static_cast<int>(format)));
          break;
      }

      const int16_t globalIndex = localIndex + mod.loadAddress;
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
        globalStackSize += module.stackSize;
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

          module.intDefTable[label] = address;
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
    for (const auto &[address, type] : mod.relocationTable) {
      std::cout << "  Address: " << address
                << ", Type: " << static_cast<int>(type) << "\n";
    }
    std::cout << "Stack Size: " << mod.stackSize << "\n";
    std::cout << "Start Address: " << mod.startAddress << "\n";
    std::cout << "Load Address: " << mod.loadAddress << "\n";
    std::cout << "\n";
  }
  std::cout << "Global Symbol Table:\n";
  for (const auto &[symbol, where] : globalSymbolTable) {
    std::cout << "Símbolo: " << symbol << " " << "Endereço: " << where.first
              << " "
              << "Módulo: " << where.second << std::endl;
  }

  std::cout << "Global Stack Size: " << globalStackSize << "\n";

  std::cout << "Linked Code: ";
  for (const auto &val : linkedCode) {
    std::cout << val << " ";
  }
  std::cout << "\n";

  std::cout << "Global Relocation Table:\n";
  for (const auto &[address, type] : globalRelocationTable) {
    std::cout << "  Address: " << address
              << ", Type: " << static_cast<int>(type) << "\n";
  }
}

#include "assembler.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

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
  locationCounter = 0;
  lineCounter = 0;

  bool foundStart = false;
  bool foundEnd = false;

  symbolTable.clear();

  std::ifstream in(asmFilePath);
  if (!in) {
    std::cerr << "Error: Could not open file " << asmFilePath << std::endl;
    return INVALID_CHARACTER;
  }

  std::string line;
  while (std::getline(in, line)) {
    ++lineCounter;

    ParseResult parseResult =
        ParseLine(line, static_cast<int16_t>(lineCounter));

    if (parseResult.exitCode != SUCCESS) {
      return parseResult.exitCode;
    }

    Instruction &instruction = parseResult.instruction;

    if (instruction.isComment || instruction.mnemonic.empty()) {
      continue;
    }

    // Tratando mnemonics
    std::string mnemonic = instruction.mnemonic;
    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(),
                   ::toupper);

    if (!assemblerInstructions.count(mnemonic)) {
      std::cerr << "Error: Invalid instruction " << mnemonic << " at line "
                << lineCounter << std::endl;
      return INVALID_INSTRUCTION;
    }

    // Tratando pseudo-instruções
    if (mnemonic == "START") {
      if (foundStart) {
        std::cerr << "Error: Multiple START directives found at line "
                  << lineCounter << std::endl;
        return SYMBOL_REDEFINITION;
      } else if (instruction.operands.size() != 1) {
        std::cerr << "Error: START directive requires one operand at line "
                  << lineCounter << std::endl;
        return SYNTAX_ERROR;
      } else if (foundEnd) {
        std::cerr << "Error: START directive found after END at line "
                  << lineCounter << std::endl;
        return SYNTAX_ERROR;
      } else {
        foundStart = true;
        locationCounter = std::stoi(instruction.operands[0]);
      }
    }

    if (mnemonic == "END") {
      foundEnd = true;
      continue;
    }

    if (mnemonic == "CONST" || mnemonic == "SPACE") {
      if (instruction.operands.size() != 1) {
        std::cerr << "Error: " << mnemonic
                  << " directive requires one operand at line " << lineCounter
                  << std::endl;
        return SYNTAX_ERROR;
      }
      locationCounter += 1;
    }

    if (mnemonic == "STACK") {
      if (instruction.operands.size() != 1) {
        std::cerr << "Error: STACK directive requires one operand at line "
                  << lineCounter << std::endl;
        return SYNTAX_ERROR;
      }
      locationCounter += std::stoi(instruction.operands[0]);
    }

    // Tratando instruções de máquina
    if (mnemonic == "ADD" || mnemonic == "BR" || mnemonic == "BRNEG" ||
        mnemonic == "BRPOS" || mnemonic == "BRZERO" || mnemonic == "CALL" ||
        mnemonic == "DIVIDE" || mnemonic == "LOAD" || mnemonic == "MULT" ||
        mnemonic == "READ" || mnemonic == "STORE" || mnemonic == "SUB" ||
        mnemonic == "WRITE") {
      if (instruction.operands.size() != 1) {
        std::cerr << "Error: " << mnemonic
                  << " instruction requires one operand at line " << lineCounter
                  << std::endl;
        return SYNTAX_ERROR;
      }
      locationCounter += 2;
    }

    if (mnemonic == "COPY") {
      if (instruction.operands.size() != 2) {
        std::cerr << "Error: COPY instruction requires two operands at line "
                  << lineCounter << std::endl;
        return SYNTAX_ERROR;
      }
      locationCounter += 3;
    }

    // Tratando Labels
    if (!instruction.label.empty()) {
      auto &sym = symbolTable[instruction.label];
      if (sym.defined) {
        std::cerr << "Error: Symbol redefinition for " << instruction.label
                  << " at line " << lineCounter << std::endl;
        return SYMBOL_REDEFINITION;
      } else {
        sym.address = static_cast<int16_t>(locationCounter);
        sym.defined = true;
      }
    }
  }
  if (!foundEnd) {
    std::cerr << "Error: No END directive found in the program." << std::endl;
    return NO_END;
  }
  return SUCCESS;
}

ParseResult ParseLine(const std::string &line, int lineNumber) {
  ParseResult result;

  if (line.empty() || line[0] == '*') {
    result.instruction.isComment = true;
    return result;
  }

  if (line.size() > 80) {
    result.exitCode = LINE_OVER_80_CHARACTERS;
    std::cerr << "Error: Line exceeds 80 characters at line " << lineNumber
              << std::endl;
    return result;
  }

  // Função auxiliar para pular espaços da linha
  auto skipSpaces = [&line](size_t &i) {
    while (i < line.size() && std::isspace(line[i])) {
      ++i;
    }
  };

  size_t idx = 0;

  // Captura da label
  if (!std::isspace(line[0])) {
    size_t start = 0;
    while (idx < line.size() && !std::isspace(line[idx])) {
      ++idx;
    }
    std::string candidateLabel = line.substr(start, idx - start);

    static const std::regex labelRegex(R"([A-Za-z_][A-Za-z0-9_]{0,7})");

    if (!std::regex_match(candidateLabel, labelRegex)) {
      result.exitCode = SYNTAX_ERROR;
      std::cerr << "Error: Invalid label '" << candidateLabel << "' at line "
                << lineNumber << std::endl;
      return result;
    }
    result.instruction.label = candidateLabel;
  }

  // Captura do mnemonic
  skipSpaces(idx);
  if (idx >= line.size()) {
    result.exitCode = SYNTAX_ERROR;
    std::cerr << "Error: Missing mnemonic at line " << lineNumber << std::endl;
    return result;
  }
  size_t opStart = idx;
  while (idx < line.size() && !std::isspace(line[idx])) {
    ++idx;
  }
  result.instruction.mnemonic = line.substr(opStart, idx - opStart);

  // Captura dos operandos
  skipSpaces(idx);
  while (idx < line.size()) {
    size_t operandStart = idx;
    while (idx < line.size() && !std::isspace(line[idx])) {
      ++idx;
    }
    result.instruction.operands.emplace_back(
        line.substr(operandStart, idx - operandStart));
    skipSpaces(idx);
  }

  if (result.instruction.operands.size() > 2) {
    result.exitCode = SYNTAX_ERROR;
    std::cerr << "Error: Too many operands at line " << lineNumber << std::endl;
    return result;
  }

  return result;
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
void Assembler::WriteListingFile() {
  std::ofstream lstFile(this->lstFilePath);
  if (!lstFile) {
    std::cerr << "Erro ao abrir o arquivo de listagem: " << this->lstFilePath
              << std::endl;
    return;
  }

  lstFile << "\nLISTAGEM DE CÓDIGO\n";
  for (const auto &line : this->listingLines) {
    lstFile << "[" << std::setw(5) << std::setfill('0') << line.address << " - "
            << line.generatedCode << "] " << std::setw(3) << std::setfill('0')
            << line.lineNumber << " - " << line.sourceCode << "\n";
  }

  if (!this->listingErrors.empty()) {
    lstFile << "\nERROS DE COMPILAÇÃO\n";
    for (const ListingError &err : this->listingErrors) {
      lstFile << "Linha " << std::setw(3) << std::setfill('0') << err.lineNumber
              << ": " << err.error << "\n";
    }
  } else {
    lstFile << "\nNENHUM ERRO DETECTADO\n";
  }

  lstFile.close();
}
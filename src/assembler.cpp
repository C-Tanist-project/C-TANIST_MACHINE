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
  AssemblerExitCode exitCode = SUCCESS;

  locationCounter = 0;
  lineCounter = 0;

  bool foundStart = false;
  bool foundEnd = false;

  symbolTable.clear();
  listingLines.clear();
  listingErrors.clear();

  std::ifstream in(asmFilePath);
  if (!in) {
    std::cerr << "Error: Could not open file " << asmFilePath << std::endl;
  }

  std::string line;
  while (std::getline(in, line)) {
    ++lineCounter;

    ParseResult parseResult =
        ParseLine(line, static_cast<int16_t>(lineCounter));

    // Inserindo erros da linha
    listingErrors.insert(listingErrors.end(), parseResult.diagnostics.begin(),
                         parseResult.diagnostics.end());

    if (parseResult.exitCode != SUCCESS && exitCode == SUCCESS) {
      exitCode = parseResult.exitCode;
    }

    listingLines.push_back(ListingLine{static_cast<int16_t>(locationCounter),
                                       "", static_cast<int16_t>(lineCounter),
                                       line});

    Instruction &instruction = parseResult.instruction;

    if (instruction.isComment || instruction.mnemonic.empty()) {
      continue;
    }

    // Tratando Labels
    if (!instruction.label.empty()) {
      auto &sym = symbolTable[instruction.label];
      if (sym.defined) {
        listingErrors.push_back(
            ListingError{static_cast<int16_t>(lineCounter),
                         "Symbol redefinition: " + instruction.label});
        if (exitCode == SUCCESS) exitCode = SYMBOL_REDEFINITION;
      } else {
        sym.address = static_cast<int16_t>(locationCounter);
        sym.defined = true;
      }
    }

    // Tratando mnemonics
    std::string mnemonic = instruction.mnemonic;
    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(),
                   ::toupper);

    if (!assemblerInstructions.count(mnemonic)) {
      listingErrors.push_back(
          ListingError{static_cast<int16_t>(lineCounter),
                       "Invalid mnemonic: " + instruction.mnemonic});
      if (exitCode == SUCCESS) exitCode = INVALID_INSTRUCTION;
      continue;
    }

    // Tratando pseudo-instruções
    if (instruction.mnemonic == "START") {
      if (instruction.operands.size() != 1) {
        listingErrors.push_back(ListingError{static_cast<int16_t>(lineCounter),
                                             "START requires one operand"});
        if (exitCode == SUCCESS) exitCode = SYNTAX_ERROR;
        continue;
      }
    }
    // Continuar tratando outras pseudo-instruções...
  }
}

ParseResult ParseLine(const std::string &line, int lineNumber) {
  ParseResult result;

  auto addError = [&result, lineNumber](AssemblerExitCode exitCode,
                                        std::string errorMessage) {
    result.diagnostics.push_back(
        ListingError{static_cast<int16_t>(lineNumber), errorMessage});
    if (result.exitCode == SUCCESS) {
      result.exitCode = exitCode;
    }
  };

  if (line.empty() || line[0] == '*') {
    result.instruction.isComment = true;
    return result;
  }

  if (line.size() > 80) {
    addError(LINE_OVER_80_CHARACTERS,
             "Error: Line exceeds 80 characters at line " +
                 std::to_string(lineNumber));
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
      addError(SYNTAX_ERROR, "Error: Invalid label format at line " +
                                 std::to_string(lineNumber));
    }
    result.instruction.label = candidateLabel;
  }

  // Captura do mnemonic
  skipSpaces(idx);
  if (idx >= line.size()) {
    addError(SYNTAX_ERROR,
             "Error: Missing mnemonic at line " + std::to_string(lineNumber));
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
    addError(SYNTAX_ERROR,
             "Error: Too many operands at line " + std::to_string(lineNumber));
  }

  return result;
}

AssemblerExitCode Assembler::SecondPass() {
  AssemblerExitCode exitCode = SUCCESS;

  // faz a (adivinha?) segunda passagem
  return exitCode;
}

// escreve o arquivo .obj com base no vetor this->objectCode
void Assembler::WriteObjectCodeFile() {}

// escreve o arquivo .lst com this->listingLines e this->listingErrors
void Assembler::WriteListingFile() {}

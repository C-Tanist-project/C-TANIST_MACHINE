#include "assembler.hpp"

#include <regex>
#include <string>

std::unordered_map<std::string, int16_t> opcodes = {
    {"ADD", 2},   {"BR", 0},    {"BRNEG", 5},   {"BRPOS", 1}, {"BRZERO", 4},
    {"CALL", 15}, {"COPY", 13}, {"DIVIDE", 10}, {"LOAD", 3},  {"MULT", 14},
    {"PUSH", 17}, {"POP", 18},  {"READ", 12},   {"RET", 16},  {"STOP", 11},
    {"SUB", 6},   {"WRITE", 8}};

Assembler::Assembler() {}

AssemblerExitCode Assembler::Assemble(const std::string &asmFilePath,
                                      const std::string &objFilePath,
                                      const std::string &lstFilePath) {
  this->asmFilePath = asmFilePath;
  this->objFilePath = objFilePath;
  this->lstFilePath = lstFilePath;

  ResetAssembler();
  this->finalExitCode = this->FirstPass();
  if (this->finalExitCode != SUCCESS)
    return finalExitCode;
  this->finalExitCode = this->SecondPass();
  if (this->finalExitCode != SUCCESS)
    return finalExitCode;
  WriteObjectCodeFile();
  WriteListingFile();
  return finalExitCode;
}
AssemblerExitCode Assembler::FirstPass() {
  locationCounter = 0;
  lineCounter = 0;

  bool foundStart = false;
  bool foundEnd = false;

  symbolTable.clear();
  literalTable.clear();

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
    // Ignorar prefixo '#' ou sufixo ',I' no mnemonic
    if (!mnemonic.empty() && mnemonic[0] == '#') {
      mnemonic = mnemonic.substr(1);
    }
    if (!mnemonic.empty() && mnemonic.size() > 2 &&
        mnemonic.substr(mnemonic.size() - 2) == ",I") {
      mnemonic = mnemonic.substr(0, mnemonic.size() - 2);
    }
    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(),
                   ::toupper);

    // Tratando intDefTable e intUseTable
    if (mnemonic == "INTUSE") {
      if (instruction.label.empty()) {
        std::cerr << "Error: INTUSE directive requires a label at line "
                  << lineCounter << std::endl;
        return SYNTAX_ERROR;
      }
      intUseTable[instruction.label] = {};
      continue;
    }

    if (mnemonic == "INTDEF") {
      if (instruction.operands.size() != 1) {
        std::cerr << "Error: INTDEF directive requires one operand at line "
                  << lineCounter << std::endl;
        return SYNTAX_ERROR;
      }

      const std::string &symbol = instruction.operands[0];
      auto &defSymData = intDefTable[symbol];
      if (defSymData.defined) {
        std::cerr << "Error: Symbol redefinition for " << symbol << " at line "
                  << lineCounter << std::endl;
        return SYMBOL_REDEFINITION;
      }
      defSymData.defined =
          true; // Simbolo definido na tabela de definições não significa que
                // foi definido na tabela de símbolos
      defSymData.address = UNRESOLVED_ADDRESS;
      defSymData.line = lineCounter;
      continue;
    }

    // Tratando Labels
    if (!instruction.label.empty()) {
      auto &symData = symbolTable[instruction.label];
      if (symData.defined) {
        std::cerr << "Error: Symbol redefinition for " << instruction.label
                  << " at line " << lineCounter << std::endl;
        return SYMBOL_REDEFINITION;
      }
      symData.address = static_cast<int16_t>(locationCounter);
      symData.defined = true;

      if (intDefTable.contains(instruction.label)) {
        intDefTable[instruction.label].address = symData.address;
      }
    }

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

    } else if (mnemonic == "END") {
      foundEnd = true;
      continue;

    } else if (mnemonic == "CONST") {
      if (instruction.operands.size() != 1) {
        std::cerr << "Error: " << mnemonic
                  << " directive requires one operand at line " << lineCounter
                  << std::endl;
        return SYNTAX_ERROR;
      }
      locationCounter += 1;

    } else if (mnemonic == "SPACE") {
      if (instruction.operands.size() != 0) {
        std::cerr << "Error: SPACE directive does not require operands at line "
                  << lineCounter << std::endl;
        return SYNTAX_ERROR;
      }
      locationCounter += 1;

    } else if (mnemonic == "STACK") {
      if (instruction.operands.size() != 1) {
        std::cerr << "Error: STACK directive requires one operand at line "
                  << lineCounter << std::endl;
        return SYNTAX_ERROR;
      }
      locationCounter += std::stoi(instruction.operands[0]);

    } else if (opcodes.contains(mnemonic)) {
      if (mnemonic == "COPY") {
        if (instruction.operands.size() != 2) {
          std::cerr << "Error: COPY instruction requires two operands at line "
                    << lineCounter << std::endl;
          return SYNTAX_ERROR;
        }
        locationCounter += 3;
      } else if (instruction.operands.size() != 1) {
        std::cerr << "Error: " << mnemonic
                  << " instruction requires one operand at line " << lineCounter
                  << std::endl;
        return SYNTAX_ERROR;
      } else {
        locationCounter += 2;
      }
    }

    // Tratando operandos
    for (auto &operand : instruction.operands) {
      if (!operand.empty() && operand[0] == '@') {
        const std::string digits = operand.substr(1);
        if (digits.empty() ||
            !std::all_of(digits.begin(), digits.end(), ::isdigit)) {
          std::cerr
              << "Error: Literal operand must be followed by a number at line "
              << lineCounter << std::endl;
          return SYNTAX_ERROR;
        }
        auto &literalData = literalTable[operand];
        if (literalData.defined) {
          std::cerr << "Error: Literal " << operand << " redefined at line "
                    << lineCounter << std::endl;
        }
      }
    }
  }

  // Fecha literal table
  int16_t poolBase = locationCounter;
  for (auto &[literal, data] : literalTable) {
    if (!data.defined) {
      data.address = poolBase++;
      data.defined = true;
    }
  }
  locationCounter = poolBase;

  // Verifica se INTDEFs foram resolvidos
  for (const auto &[label, defData] : intDefTable) {
    if (defData.address == -1) {
      std::cerr << "Error: INTDEF " << label << " not defined at line "
                << defData.line << std::endl;
      return SYMBOL_UNDEFINED;
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
  this->lineCounter = 0;

  std::string label, opcode, operand1, operand2;

  std::string line;
  std::ifstream file(this->asmFilePath);

  while (std::getline(file, line)) {
    ++this->lineCounter;

    if (!(line.empty() || line[0] == '*')) {
      std::istringstream lineStream(line);
      std::string firstToken = [&]() -> std::string {
        std::streampos pos = lineStream.tellg();
        std::string first;
        lineStream >> first;

        lineStream.seekg(pos);

        return first;
      }();

      label = "";
      opcode = "";
      operand1 = "";
      operand2 = "";
      if (!assemblerInstructions.contains(firstToken)) {
        lineStream >> label >> opcode >> operand1 >> operand2;
      } else {
        lineStream >> opcode >> operand1 >> operand2;
      }

      if (opcodes.contains(opcode)) {
        objectCode.push_back(opcodes[opcode]);
        int16_t finalOpCode = opcodes[opcode];

        size_t opcodeIdx = objectCode.size() - 1;
        std::string generatedCodeForLst = "";
        std::string sourceCodeForLst;

        sourceCodeForLst += opcode;

        auto solveOperand = [&](std::string &operand, int whichOne) {
          sourceCodeForLst += " ";
          sourceCodeForLst += operand;

          // INDIRETO
          if (operand.back() == 'I') {
            finalOpCode += whichOne;
            objectCode[opcodeIdx] = finalOpCode;
            operand.pop_back();
            if (!symbolTable.contains(operand)) {
              exitCode = SYMBOL_UNDEFINED;
              return;
            }
            relocationTable.emplace(static_cast<int16_t>(objectCode.size()),
                                    OperandFormat::INDIRECT);
            objectCode.push_back(symbolTable[operand].address);
            generatedCodeForLst += " ";
            generatedCodeForLst += std::to_string(symbolTable[operand].address);
            return;

            // IMEDIATO
          } else if (operand[0] == '#') {
            if (opcode == "COPY" && whichOne == 32) {
              exitCode = INVALID_CHARACTER;
              return;
            }
            finalOpCode += 128;
            objectCode[opcodeIdx] = finalOpCode;
            operand = operand.substr(1);
            if (operand[0] == 'H') {
              operand = operand.substr(2, operand.size() - 3);
            }
            objectCode.push_back(static_cast<int16_t>(std::stoi(operand)));
            generatedCodeForLst += " ";
            generatedCodeForLst += operand;
            return;

            // DIRETO
          } else {
            if (!symbolTable.contains(operand) &&
                !literalTable.contains(operand) &&
                !intUseTable.contains(operand)) {
              exitCode = SYMBOL_UNDEFINED;
              return;
            }
            int16_t address;
            std::string addressToLst;
            if (operand[0] == '@') { // literal
              address = literalTable[operand].address;
              addressToLst = std::to_string(literalTable[operand].address);
              relocationTable.emplace(static_cast<int16_t>(objectCode.size()),
                                      OperandFormat::DIRECT);
            } else if (symbolTable.contains(operand)) { // símbolo local
              address = symbolTable[operand].address;
              addressToLst = std::to_string(symbolTable[operand].address);
              relocationTable.emplace(static_cast<int16_t>(objectCode.size()),
                                      OperandFormat::DIRECT);
            } else { // símbolo definido em outro módulo
              address = 0;
              addressToLst = "0";
              intUseTable[operand].push_back(objectCode.size());
            }
            objectCode.push_back(address);
            generatedCodeForLst += " ";
            generatedCodeForLst += addressToLst;
          }
        };
        if (!operand1.empty())
          solveOperand(operand1, 32);
        if (!operand2.empty())
          solveOperand(operand2, 64);
        if (exitCode != SUCCESS)
          return exitCode;
        ListingLine listingLine;
        listingLine.address = opcodeIdx + 1;
        listingLine.generatedCode =
            std::to_string(objectCode[opcodeIdx]) + generatedCodeForLst;
        listingLine.lineNumber = lineCounter;
        listingLine.sourceCode = sourceCodeForLst;

        listingLines.push_back(listingLine);
      } else if (opcode == "STACK")
        stackSize = std::stoi(operand1);
      else if (opcode == "CONST")
        objectCode.push_back(static_cast<int16_t>(std::stoi(operand1)));
      else if (opcode == "SPACE") {
        if (operand1 == "")
          operand1 = "1";
        objectCode.insert(objectCode.end(), std::stoi(operand1), 0);
      } else
        continue;
    }
  }
  file.close();
  // resolvendo literais
  for (auto &[label, _] : literalTable) {
    int16_t value = std::stoi(label.substr(1));
    objectCode.push_back(value);
  }
  return exitCode;
}

// escreve o arquivo .obj com base no vetor this->objectCode
void Assembler::WriteObjectCodeFile() {
  std::ofstream objFile(this->objFilePath, std::ios::binary);
  if (!objFile) {
    std::cerr << "Erro ao abrir o arquivo de código objeto: "
              << this->objFilePath << std::endl;
  }

  // STACK_SIZE
  int16_t _stackSizeSection = static_cast<int16_t>(ObjSectionType::STACK_SIZE);
  objFile.write(reinterpret_cast<const char *>(&_stackSizeSection),
                sizeof(int16_t));
  objFile.write(reinterpret_cast<const char *>(&this->stackSize),
                sizeof(int16_t));

  // INTDEF
  int16_t intDefSection = static_cast<int16_t>(ObjSectionType::INTDEF);
  objFile.write(reinterpret_cast<const char *>(&intDefSection),
                sizeof(int16_t));
  int16_t defCount = static_cast<int16_t>(this->intDefTable.size());
  objFile.write(reinterpret_cast<const char *>(&defCount), sizeof(int16_t));

  for (const auto &[label, addr] : this->intDefTable) {
    int16_t labelLen = static_cast<int16_t>(label.size());
    objFile.write(reinterpret_cast<const char *>(&labelLen), sizeof(int16_t));
    objFile.write(label.data(), labelLen);
    objFile.write(reinterpret_cast<const char *>(&addr), sizeof(int16_t));
  }

  // INTUSE
  int16_t intUseSection = static_cast<int16_t>(ObjSectionType::INTUSE);
  objFile.write(reinterpret_cast<const char *>(&intUseSection),
                sizeof(int16_t));
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
  int16_t codeSection = static_cast<int16_t>(ObjSectionType::CODE);
  objFile.write(reinterpret_cast<const char *>(&codeSection), sizeof(int16_t));
  int16_t codeSize = static_cast<int16_t>(this->objectCode.size());
  objFile.write(reinterpret_cast<const char *>(&codeSize), sizeof(int16_t));
  objFile.write(reinterpret_cast<const char *>(this->objectCode.data()),
                this->objectCode.size() * sizeof(int16_t));

  // RELOCATION (nova seção)
  int16_t relocationSection = static_cast<int16_t>(ObjSectionType::RELOCATION);
  objFile.write(reinterpret_cast<const char *>(&relocationSection),
                sizeof(int16_t));
  int16_t relocCount = static_cast<int16_t>(this->relocationTable.size());
  objFile.write(reinterpret_cast<const char *>(&relocCount), sizeof(int16_t));

  for (const auto &[address, type] : this->relocationTable) {
    objFile.write(reinterpret_cast<const char *>(&address), sizeof(int16_t));
    int16_t typeVal = static_cast<int16_t>(type); // DIRECT ou INDIRECT
    objFile.write(reinterpret_cast<const char *>(&typeVal), sizeof(int16_t));
  }

  // END
  int16_t endSection = static_cast<int16_t>(ObjSectionType::END);
  objFile.write(reinterpret_cast<const char *>(&endSection), sizeof(int16_t));
  objFile.close();
}

// escreve o arquivo .lst com this->listingLines e this->listingErrors
void Assembler::WriteListingFile() {

  std::ofstream lstFile(this->lstFilePath);

  if (!lstFile.is_open()) {
    std::cerr << "Erro ao criar o arquivo de listagem: " << this->lstFilePath
              << std::endl;
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

void Assembler::ResetAssembler() {
  locationCounter = 0;
  lineCounter = 0;
  objectCode.clear();
  listingLines.clear();
  listingErrors.clear();
  intDefTable.clear();
  intUseTable.clear();
  stackSize = 0;
  symbolTable.clear();
  literalTable.clear();
  relocationTable.clear();
}

void Assembler::CallAssembler(std::vector<std::string> paths) {
  for (const auto &path : paths) {
    size_t lastSlash = path.rfind('/');
    std::string fileName =
        path.substr(lastSlash + 1, path.find_last_of('.') - (lastSlash + 1));
    std::string asmFilePath = path;
    std::string objFilePath = "./obj/" + fileName + ".obj";
    std::string lstFilePath = "./lst/" + fileName + ".lst";

    Assemble(asmFilePath, objFilePath, lstFilePath);
  }
}

#include "assembler.hpp"

#include <filesystem>
#include <regex>
#include <string>

std::unordered_map<std::string, Registers> regs = {
    {"ACC", ACC}, {"R0", R0}, {"R1", R1}};
std::unordered_map<AssemblerExitCode, std::string> errorMessages = {
    {SUCCESS, "Sem erros"},
    {INVALID_CHARACTER, "Lexema inválido"},
    {LINE_OVER_80_CHARACTERS, "Linha muito longa (+80 caracteres)"},
    {INVALID_DIGIT, "Dígito inválido para a base escolhida"},
    {UNEXPECTED_EOL, "Fim de instrução inesperado"},
    {OUT_OF_BOUNDS, "Valor fora do intervalo de int16_t"},
    {SYNTAX_ERROR, "Erro de sintaxe"},
    {SYMBOL_REDEFINITION, "Símbolo redefinido"},
    {SYMBOL_UNDEFINED, "Símbolo indefinido"},
    {INVALID_INSTRUCTION, "Instrução inválida"},
    {NO_END, "Faltou END no programa"},
};

std::unordered_map<std::string, int16_t> opcodes = {
    {"ADD", 2},   {"BR", 0},    {"BRNEG", 5},   {"BRPOS", 1}, {"BRZERO", 4},
    {"CALL", 15}, {"COPY", 13}, {"DIVIDE", 10}, {"LOAD", 3},  {"MULT", 14},
    {"PUSH", 17}, {"POP", 18},  {"READ", 12},   {"RET", 16},  {"STOP", 11},
    {"SUB", 6},   {"WRITE", 8}, {"STORE", 7}};

Assembler::Assembler() {}

void Assembler::Assemble(const std::string &asmFilePath,
                         const std::string &objFilePath,
                         const std::string &lstFilePath) {
  this->asmFilePath = asmFilePath;
  this->objFilePath = objFilePath;
  this->lstFilePath = lstFilePath;

  ResetAssembler();
  this->assemblingStatus = FirstPass();
  if (this->assemblingStatus.exitCode == SUCCESS) {
    std::cout << "Primeiro passo fucnional" << std::endl;
    this->assemblingStatus = SecondPass();
  }
  WriteObjectCodeFile();
  WriteListingFile();
  return;
}

AssemblingStatus Assembler::FirstPass() {
  AssemblingStatus status;
  locationCounter = 0;
  lineCounter = 0;

  bool foundStart = false;

  bool foundEnd = false;

  symbolTable.clear();
  literalTable.clear();

  std::ifstream in(asmFilePath);
  if (!in) { // erro ao abrir arquivo pode entrar na lista de erros do
             // assembler.
    std::cerr << "Error: Could not open file " << asmFilePath << std::endl;
    return status;
  }

  std::string line;
  while (std::getline(in, line)) {
    ++lineCounter;

    ParseResult parseResult =
        ParseLine(line, static_cast<int16_t>(lineCounter));

    if (parseResult.lineStatus.exitCode != SUCCESS) {
      return parseResult.lineStatus;
    }

    Instruction &instruction = parseResult.instruction;

    if (instruction.isComment || instruction.mnemonic.empty()) {
      continue;
    }

    // Tratando mnemonics
    std::string mnemonic = instruction.mnemonic;
    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(),
                   ::toupper);

    // Tratando intDefTable e intUseTable
    if (mnemonic == "INTUSE") {
      if (instruction.label.empty()) {
        return buildError(lineCounter, mnemonic,
                          SYNTAX_ERROR); // INTUSE ERROR (ter que mudar)
      }
      intUseTable[instruction.label] = {};
      continue;
    }

    if (mnemonic == "INTDEF") {
      if (instruction.operands.size() != 1) {
        return buildError(lineCounter, instruction.operands[0],
                          SYNTAX_ERROR); // INTDEF ERROR (ter que mudar)
      }

      const std::string &symbol = instruction.operands[0];
      auto &defSymData = intDefTable[symbol];
      if (defSymData.defined) {
        return buildError(lineCounter, instruction.label, SYMBOL_REDEFINITION);
      }
      defSymData.defined =
          true;  // Simbolo definido na tabela de definições não significa que
                 // foi definido na tabela de símbolos
      defSymData.address = UNRESOLVED_ADDRESS;
      defSymData.line = lineCounter;
      continue;
    }

    // Tratando Labels
    if (!instruction.label.empty()) {
      auto &symData = symbolTable[instruction.label];
      if (symData.defined) {
        return buildError(lineCounter, instruction.label, SYMBOL_REDEFINITION);
      }
      symData.address = static_cast<int16_t>(locationCounter);
      symData.defined = true;

      if (intDefTable.contains(instruction.label)) {
        intDefTable[instruction.label].address = symData.address;
      }
    }

    if (!assemblerInstructions.count(mnemonic)) {
      return buildError(lineCounter, instruction.label, INVALID_INSTRUCTION);
    }

    // Tratando pseudo-instruções
    if (mnemonic == "START") {
      if (foundStart) {
        return buildError(
            lineCounter, mnemonic,
            SYMBOL_REDEFINITION); // Erro de múltiplos START (ter que mudar)
      } else if (instruction.operands.size() != 1) {
        return buildError(lineCounter, mnemonic,
                          SYNTAX_ERROR); // Falta operando no START
      } else if (foundEnd) {
        return buildError(lineCounter, mnemonic,
                          SYNTAX_ERROR); // erro de start dps do end
      } else {
        foundStart = true;
        this->moduleName = instruction.operands[0];
      }

    } else if (mnemonic == "END") {
      foundEnd = true;
      continue;

    } else if (mnemonic == "CONST") {
      if (instruction.operands.size() != 1) {
        return buildError(lineCounter, mnemonic,
                          SYNTAX_ERROR); // Const sem operando
      }
      locationCounter += 1;

    } else if (mnemonic == "SPACE") {
      if (instruction.operands.size() < 0 || instruction.operands.size() > 2) {
        return buildError(lineCounter, mnemonic, SYNTAX_ERROR);
      }
      locationCounter += 1;

    } else if (mnemonic == "STACK") {
      if (instruction.operands.size() != 1) {
        return buildError(lineCounter, mnemonic,
                          SYNTAX_ERROR); // STACK sem operando
      }

    } else if (opcodes.contains(mnemonic)) {
      if (mnemonic == "COPY") {
        if (instruction.operands.size() != 2) {
          return buildError(lineCounter, mnemonic,
                            SYNTAX_ERROR); // Faltando operandos em COPY
        }
        locationCounter += 3;
      } else if (mnemonic == "RET" || mnemonic == "STOP") {
        if (instruction.operands.size() > 0) {
          return buildError(lineCounter, mnemonic,
                            SYNTAX_ERROR); // Muitos operandos em RET ou STOP
        }
        locationCounter += 1;
      } else {
        if (instruction.operands.size() != 1) {
          return buildError(lineCounter, mnemonic,
                            SYNTAX_ERROR); // Agora sim isso tá certo
        }
        locationCounter += 2;
      }
    }

    // Tratando operandos
    for (auto &operand : instruction.operands) {
      if (operand.empty())
        continue;

      if (operand[0] == '#') {
        operand = operand.substr(1);
      }

      if (operand.size() > 2 && operand.substr(operand.size() - 2) == ",I") {
        operand = operand.substr(0, operand.size() - 2);
      }

      if (!operand.empty() && operand[0] == '@') {
        const std::string digits = operand.substr(1);
        if (digits.empty() ||
            !std::all_of(digits.begin(), digits.end(), ::isdigit)) {
          return buildError(lineCounter, operand,
                            SYNTAX_ERROR); // Literal sem número
        }

        auto &literalData = literalTable[operand];
        if (literalData.defined) {
          return buildError(lineCounter, operand,
                            SYMBOL_REDEFINITION); // redefinição de literal
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
      return buildError(lineCounter, label,
                        SYMBOL_UNDEFINED); // INTDEF não definido (????????/)
    }
  }

  if (!foundEnd) {
    std::cerr << "Error: No END directive found in the program." << std::endl;
    return buildError(lineCounter, "", NO_END);
  }
  return status;
}

ParseResult Assembler::ParseLine(const std::string &line, int lineNumber) {
  ParseResult result;
  result.lineStatus.exitCode = SUCCESS;

  if (line.empty() || line[0] == '*') {
    result.instruction.isComment = true;
    return result;
  }

  if (line.size() > 80) {
    result.lineStatus = buildError(lineNumber, "", LINE_OVER_80_CHARACTERS);
    return result;
  }

  // Função auxiliar para pular espaços da linha
  auto skipSpaces = [&line](size_t &i) {
    while (i < line.size() && std::isspace(line[i])) {
      ++i;
    }
  };

  size_t idx = 0;
  // Captura da primeira palavra (sem depender de indentação)
  skipSpaces(idx);
  size_t start = idx;
  while (idx < line.size() && !std::isspace(line[idx])) {
    ++idx;
  }
  std::string firstWord = line.substr(start, idx - start);

  if (assemblerInstructions.contains(firstWord)) {
    // É um mnemonico -> não tem label
    result.instruction.mnemonic = firstWord;
  } else {
    // Pode ser label
    static const std::regex labelRegex(R"([A-Za-z_][A-Za-z0-9_]{0,7})");

    if (!std::regex_match(firstWord, labelRegex)) {
      result.lineStatus = buildError(
          lineNumber, firstWord,
          LINE_OVER_80_CHARACTERS); // Verificar se esse erro é esse erro mesmo
      return result;
    }
    result.instruction.label = firstWord;

    // Agora capturar o mnemonico
    skipSpaces(idx);
    if (idx >= line.size()) {
      result.lineStatus = buildError(
          lineNumber, firstWord,
          LINE_OVER_80_CHARACTERS); // Verificar se esse erro é esse erro mesmo
      return result;
    }
    size_t opStart = idx;
    while (idx < line.size() && !std::isspace(line[idx])) {
      ++idx;
    }
    result.instruction.mnemonic = line.substr(opStart, idx - opStart);
  }

  // Captura dos operandos
  skipSpaces(idx);
  while (idx < line.size()) {
    if (line[idx] == ';') {
      break;
    }

    size_t operandStart = idx;
    while (idx < line.size() && !std::isspace(line[idx]) && line[idx] != ';') {
      ++idx;
    }

    result.instruction.operands.emplace_back(
        line.substr(operandStart, idx - operandStart));

    skipSpaces(idx);
  }

  if (result.instruction.operands.size() > 2) {
    result.lineStatus = buildError(
        lineNumber, firstWord,
        LINE_OVER_80_CHARACTERS); // Verificar se esse erro é esse erro mesmo
    return result;
  }

  return result;
}

AssemblingStatus Assembler::SecondPass() {
  AssemblingStatus status;
  this->lineCounter = 0;

  std::string label, opcode, operand1, operand2;

  std::string line;
  std::ifstream file(this->asmFilePath);

  if (!file) {
    std::cerr << "Error opening file" << this->asmFilePath << std::endl;
    return buildError(0, "file", SYNTAX_ERROR);
  }

  while (std::getline(file, line)) {
    ++this->lineCounter;

    if (!(line.empty() || line[0] == '*')) {
      std::string noCommentLine = line;
      size_t commentPos = noCommentLine.find(';');
      if (commentPos != std::string::npos) {
        noCommentLine = noCommentLine.substr(0, commentPos);
      }

      std::istringstream lineStream(noCommentLine);
      std::string firstToken = [&]() -> std::string {
        std::streampos pos = lineStream.tellg();
        std::string first;
        lineStream >> first;

        lineStream.seekg(pos);

        return first;
      }();

      std::string firstTokenToUp = firstToken;
      std::transform(firstTokenToUp.begin(), firstTokenToUp.end(),
                     firstTokenToUp.begin(),
                     [](unsigned char c) { return std::toupper(c); });
      label = "";
      opcode = "";
      operand1 = "";
      operand2 = "";

      if (!assemblerInstructions.contains(firstTokenToUp)) {
        lineStream >> label >> opcode >> operand1 >> operand2;
      } else {
        lineStream >> opcode >> operand1 >> operand2;
      }
      std::transform(opcode.begin(), opcode.end(), opcode.begin(),
                     [](unsigned char c) { return std::toupper(c); });

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

          if (opcode == "PUSH" || opcode == "POP") {
            if (!regs.contains(operand)) {
              status = buildError(lineCounter, operand,
                                  SYNTAX_ERROR); // erro de operando inválido
              return;
            }
            objectCode.push_back(regs.at(operand));
            generatedCodeForLst += " ";
            generatedCodeForLst += operand;
          }
          // INDIRETO
          else if (operand.size() >= 2 &&
                   operand.substr(operand.size() - 2) == ",I") {
            finalOpCode += whichOne;
            objectCode[opcodeIdx] = finalOpCode;
            operand = operand.substr(0, operand.size() - 2);

            if (!symbolTable.contains(operand)) {
              status = buildError(lineCounter, operand, SYMBOL_UNDEFINED);
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
            if (opcode == "BR" || opcode == "BRNEG" || opcode == "BRPOS" ||
                opcode == "BRZERO" || opcode == "CALL" || opcode == "READ" ||
                opcode == "STORE") {
              status = buildError(lineCounter, operand, INVALID_CHARACTER);
              return;
            }
            if (opcode == "COPY" && whichOne == 32) {
              status = buildError(
                  lineCounter, operand,
                  INVALID_CHARACTER); // verificar se esse erro é o certo
              return;
            }
            finalOpCode += 128;
            objectCode[opcodeIdx] = finalOpCode;
            operand = operand.substr(1);
            if (operand[0] == 'H') {
              operand = operand.substr(2, operand.size() - 3);
              objectCode.push_back(
                  static_cast<int16_t>(std::stoi(operand, nullptr, 16)));
            } else {
              objectCode.push_back(static_cast<int16_t>(std::stoi(operand)));
            }
            generatedCodeForLst += " ";
            generatedCodeForLst += operand;
            return;

            // DIRETO
          } else {
            if (!symbolTable.contains(operand) &&
                !literalTable.contains(operand) &&
                !intUseTable.contains(operand)) {
              status = buildError(lineCounter, operand, SYMBOL_UNDEFINED);
              return;
            }
            int16_t address;
            std::string addressToLst;
            if (operand[0] == '@') {  // literal
              address = literalTable[operand].address;
              addressToLst = std::to_string(literalTable[operand].address);
              relocationTable.emplace(static_cast<int16_t>(objectCode.size()),
                                      OperandFormat::DIRECT);
            } else if (symbolTable.contains(operand)) {  // símbolo local
              address = symbolTable[operand].address;
              addressToLst = std::to_string(symbolTable[operand].address);
              relocationTable.emplace(static_cast<int16_t>(objectCode.size()),
                                      OperandFormat::DIRECT);
            } else {  // símbolo definido em outro módulo
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
        if (status.exitCode != SUCCESS)
          return status;

        ListingLine listingLine;
        listingLine.address = opcodeIdx + 1;
        listingLine.generatedCode =
            std::to_string(objectCode[opcodeIdx]) + generatedCodeForLst;
        listingLine.lineNumber = lineCounter;
        listingLine.sourceCode = sourceCodeForLst;

        listingLines.push_back(listingLine);
      } else if (opcode == "STACK")
        stackSize = std::stoi(operand1);
      else if (opcode == "CONST") {
        if (operand1[0] == 'H') {
          operand1 = operand1.substr(2, operand1.size() - 3);
          objectCode.push_back(
              static_cast<int16_t>(std::stoi(operand1, NULL, 16)));
          continue;
        }
        objectCode.push_back(static_cast<int16_t>(std::stoi(operand1)));
      } else if (opcode == "SPACE") {
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
  return status;
}

// escreve o arquivo .obj com base no vetor this->objectCode
void Assembler::WriteObjectCodeFile() {
  std::ofstream objFile(this->objFilePath, std::ios::binary);
  if (!objFile) {
    std::cerr << "Erro ao abrir o arquivo de código objeto: "
              << this->objFilePath << std::endl;
  }

  // NAME
  int16_t nameSection = static_cast<int16_t>(ObjSectionType::NAME);
  objFile.write(reinterpret_cast<const char *>(&nameSection), sizeof(int16_t));

  int16_t nameLen = static_cast<int16_t>(this->moduleName.size());
  objFile.write(reinterpret_cast<const char *>(&nameLen), sizeof(int16_t));
  objFile.write(this->moduleName.data(), nameLen);

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

  // RELOCATION
  int16_t relocationSection = static_cast<int16_t>(ObjSectionType::RELOCATION);
  objFile.write(reinterpret_cast<const char *>(&relocationSection),
                sizeof(int16_t));
  int16_t relocCount = static_cast<int16_t>(this->relocationTable.size());
  objFile.write(reinterpret_cast<const char *>(&relocCount), sizeof(int16_t));

  for (const auto &[address, type] : this->relocationTable) {
    objFile.write(reinterpret_cast<const char *>(&address), sizeof(int16_t));
    int16_t typeVal = static_cast<int16_t>(type);  // DIRECT ou INDIRECT
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

  if (this->assemblingStatus.exitCode != SUCCESS) {
    lstFile << "\nERROS DE COMPILAÇÃO\n";
    lstFile << "Na linha " << assemblingStatus.lineNumber
            << "\nErro: " << assemblingStatus.erro
            << "\nPróximo ao token: " << assemblingStatus.badToken;
  } else {
    lstFile << "\nNENHUM ERRO DETECTADO\n";
  }

  lstFile.close();
}

AssemblingStatus Assembler::buildError(int16_t line, const std::string &token,
                                       AssemblerExitCode statusCode) {
  AssemblingStatus status;
  status.lineNumber = line;
  status.badToken = token;
  status.exitCode = statusCode;
  status.erro = errorMessages.at(statusCode);
  return status;
}

void Assembler::ResetAssembler() {
  locationCounter = 0;
  lineCounter = 0;
  moduleName = "";
  objectCode.clear();
  listingLines.clear();
  intDefTable.clear();
  intUseTable.clear();
  stackSize = 0;
  symbolTable.clear();
  literalTable.clear();
  relocationTable.clear();
}

void Assembler::Pass(std::filesystem::path &projectFolder) {
  std::filesystem::path inputPath = projectFolder / "ASM";
  std::filesystem::path outputPath = projectFolder / "OBJ";
  std::filesystem::path listingPath = projectFolder / "LST";

  for (const auto &entry : std::filesystem::directory_iterator(inputPath)) {
    if (std::filesystem::is_regular_file(entry.status())) {
      std::cout << entry.path() << std::endl;
      std::filesystem::path currentFile(entry.path());
      Assemble(currentFile.string(),
               (outputPath / currentFile.filename().replace_extension(".OBJ"))
                   .string(),
               (listingPath / currentFile.filename().replace_extension(".LST"))
                   .string());
    }
  }
}

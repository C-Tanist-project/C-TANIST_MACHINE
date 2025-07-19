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
  WriteObjectCodeFile();
  WriteListingFile();
  return finalExitCode;
}

AssemblerExitCode Assembler::FirstPass() { return SUCCESS; }

std::unordered_map<std::string, int16_t> opcodes = {
    {"ADD", 3},   {"BR", 0},    {"BRNEG", 5},   {"BRPOS", 1}, {"BRZERO", 4},
    {"CALL", 15}, {"COPY", 13}, {"DIVIDE", 10}, {"LOAD", 3},  {"MULT", 14},
    {"PUSH", 17}, {"POP", 18},  {"READ", 12},   {"RET", 16},  {"STOP", 11},
    {"SUB", 6},   {"WRITE", 8}};

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
          if (operand.back() == 'I') {
            finalOpCode += whichOne;
            objectCode[opcodeIdx] = finalOpCode;
            operand.pop_back();
            if (!symbolTable.contains(operand)) {
              exitCode = SYMBOL_UNDEFINED;
              return;
            }
            objectCode.push_back(symbolTable[operand].address);
            generatedCodeForLst += " ";
            generatedCodeForLst += std::to_string(symbolTable[operand].address);
            return;

          } else if (operand[0] == '#') {
            if (opcode == "COPY" && whichOne == 32) {
              exitCode = INVALID_CHARACTER;
              return;
            }
            finalOpCode += 128;
            objectCode[opcodeIdx] = finalOpCode;
            operand = operand.substr(1);
            objectCode.push_back(static_cast<int16_t>(std::stoi(operand)));
            generatedCodeForLst += " ";
            generatedCodeForLst += operand;

            return;

          } else {
            if (!symbolTable.contains(operand)) {
              exitCode = SYMBOL_UNDEFINED;
              return;
            }
            objectCode.push_back(symbolTable[operand].address);
            generatedCodeForLst += " ";
            generatedCodeForLst += std::to_string(symbolTable[operand].address);
          }
        };
        if (!operand1.empty()) solveOperand(operand1, 32);
        if (!operand2.empty()) solveOperand(operand2, 64);

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
      else if (opcode == "SPACE")
        // for (int i = 0; i < std::stoi(operand1); ++i)
        // objectCode.push_back(0);
        objectCode.insert(objectCode.end(), std::stoi(operand1), 0);
      else
        continue;
    }
  }

  file.close();
  return exitCode;
}
// Não parar a compilação com erro. Adicionar a listingerrors a linha que teve
// erro e o tipo de erro
// resolver números

//
// escreve o arquivo .obj com base no vetor this->objectCode
void Assembler::WriteObjectCodeFile() {}

// escreve o arquivo .lst com this->listingLines e this->listingErrors
void Assembler::WriteListingFile() {}
#include "assembler.hpp"

#include <fstream>
#include <iostream>
#include <string>

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
  bool FoundEnd = false;
  symbolTable.clear();
  listingLines.clear();
  listingErrors.clear();

  std::ifstream in(asmFilePath);
  if (!in) {
  }
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

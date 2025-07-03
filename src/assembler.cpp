#include "assembler.hpp"

#include <fstream>

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
    // faz a primeira passagem

    return exitCode;
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
        // Sei q idealmente isso nunca falha mas se falhar qual erro eu devo
        // colocar?
        //  this->finalExitCode = FILE_ERROR;
        return;
    }

    if (!this->objectCode.empty()) {
        objFile.write(reinterpret_cast<const char *>(this->objectCode.data()),
                      this->objectCode.size() * sizeof(int16_t));
    }
}

// escreve o arquivo .lst com this->listingLines e this->listingErrors
void Assembler::WriteListingFile() {}

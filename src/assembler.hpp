#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "types.hpp"

struct ListingLine {
  int16_t address;
  std::string generatedCode;
  int16_t lineNumber;
  std::string sourceCode;
};

struct ListingError {
  int16_t lineNumber;
  std::string error;
};

struct AssemblerSymbolData {
  int16_t address;
  bool defined = false;
};

struct AssemblerIntDefData {
  int16_t address;
  bool defined = false;
  int16_t line;
};

struct AssemblerLiteralData {
  int16_t address;
  bool defined = false;
};

struct Instruction {
  int16_t lineNumber;
  std::string mnemonic;
  std::vector<std::string> operands;
  std::string label;
  bool isComment = false;
};

struct ParseResult {
  AssemblerExitCode exitCode = SUCCESS;
  Instruction instruction;
};

ParseResult ParseLine(const std::string &line, int lineNumber);

class Assembler {
 private:
  static inline const std::unordered_set<std::string> assemblerInstructions = {
      // instruções de máquina
      "ADD", "BR", "BRNEG", "BRPOS", "BRZERO", "CALL", "COPY", "DIVIDE", "LOAD",
      "MULT", "PUSH", "POP", "READ", "RET", "STOP", "SUB", "WRITE",
      // pseudo-instruções
      "START", "END", "INTDEF", "INTUSE", "CONST", "SPACE", "STACK"};

  int locationCounter;
  int lineCounter;

  AssemblerExitCode finalExitCode;

  // caminho do programa de entrada
  std::string asmFilePath;
  // caminho do código objeto e listagem
  std::string objFilePath;
  std::string lstFilePath;

  // código de máquina que vai entrar na memória
  std::vector<int16_t> objectCode;
  // código gerado | código fonte, ver formato na especificação
  std::vector<ListingLine> listingLines;
  // inserir erros de montagem aqui (não sei se precisa ser um vetor)
  std::vector<ListingError> listingErrors;
  std::unordered_map<std::string, AssemblerIntDefData>
      intDefTable;        // tabela de simbolos definidos no modulo
  int16_t stackSize = 0;  // tamanho da pilha
  std::unordered_map<std::string, std::vector<int16_t>>
      intUseTable;  // tabela de simbolos usados no modulo

  // tabela de símbolos
  std::unordered_map<std::string, AssemblerSymbolData> symbolTable;
  // tabela de literais
  std::unordered_map<std::string, AssemblerLiteralData> literalTable;
  AssemblerExitCode FirstPass();
  AssemblerExitCode SecondPass();
  void WriteObjectCodeFile();
  void WriteListingFile();

 public:
  Assembler(const std::string &asmFilePath, const std::string &objFilePath,
            const std::string &lstFilePath);
  AssemblerExitCode Assemble();
};

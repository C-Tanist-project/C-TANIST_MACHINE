#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef enum {
  SUCCESS = 0,        // DEU BOM!
  INVALID_CHARACTER,  // lexema inválido (só vale ASCII? qq o ferrugem quer?)
  LINE_OVER_80_CHARACTERS,  // linha muito longa (+80 chars)
  INVALID_DIGIT,        // caracter inválido para a base escolhida (ex: "2" bin)
  UNEXPECTED_EOL,       // delimitador de fim de instrução inválido
  OUT_OF_BOUNDS,        // valor maior do que cabe em int16_t
  SYNTAX_ERROR,         // falta/excesso de operandos, label mal formada
  SYMBOL_REDEFINITION,  // referência simbólica com mais de uma definição
  SYMBOL_UNDEFINED,     // referência simbólica não definida
  INVALID_INSTRUCTION,  // mnemônico não corresponde a nenhuma instrução
  NO_END,               // faltou "END" no programa
} AssemblerExitCode;

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

  // tabela de símbolos
  std::unordered_map<std::string, AssemblerSymbolData> symbolTable;
  // código de máquina que vai entrar na memória
  std::vector<int16_t> objectCode;
  // código gerado | código fonte, ver formato na especificação
  std::vector<ListingLine> listingLines;
  // inserir erros de montagem aqui (não sei se precisa ser um vetor)
  std::vector<ListingError> listingErrors;

  AssemblerExitCode FirstPass();
  AssemblerExitCode SecondPass();

 public:
  Assembler(const std::string &asmFilePath, const std::string &objFilePath,
            const std::string &lstFilePath);
  AssemblerExitCode Assemble();
  void WriteObjectCodeFile();
  void WriteListingFile();
};

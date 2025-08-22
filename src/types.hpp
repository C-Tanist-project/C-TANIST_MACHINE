#pragma once

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <queue>
#include <stack>
#include <string>

typedef struct vmstatestruct {
  int16_t memory[500] = {0};

  std::atomic<float> clockSpeed = 1.0f;

  int16_t pc, sp, acc, mop, ri, re, r0, r1, inputValue;

  std::stack<int16_t> updatedMemoryAddresses;
  std::queue<std::string> consoleMessages;
  std::mutex mutex, consoleMutex;

  std::atomic<bool> isHalted{false}, isRunning{false}, hasError{false},
      waitingForInput{false};
} VMState;

typedef enum { FINISH, RUN, STEP, STOP, PAUSE, CLOSE, NONE } VMControls;

typedef enum { ACC, R0, R1 } Registers;

typedef enum { IMMEDIATE = 0, DIRECT = 1, INDIRECT = 2 } OperandFormat;

constexpr int16_t UNRESOLVED_ADDRESS = -1;

typedef enum {
  OP_BR = 0,
  OP_BRPOS = 1,
  OP_ADD = 2,
  OP_LOAD = 3,
  OP_BRZERO = 4,
  OP_BRNEG = 5,
  OP_SUB = 6,
  OP_STORE = 7,
  OP_WRITE = 8,
  // OP_UNHA = 9,
  OP_DIVIDE = 10,
  OP_STOP = 11,
  OP_READ = 12,
  OP_COPY = 13,
  OP_MULT = 14,
  OP_CALL = 15,
  OP_RET = 16,
  OP_PUSH = 17,
  OP_POP = 18,
} Opcode;

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

enum class ObjSectionType : int16_t {
  NAME = 0x00,
  STACK_SIZE = 0x01,
  INTDEF = 0x02,
  INTUSE = 0x03,
  CODE = 0x04,
  RELOCATION = 0x05,
  END = 0xFF
};
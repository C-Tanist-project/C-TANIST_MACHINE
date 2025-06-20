#ifndef H_VM
#define H_VM

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <syncstream>
#include <thread>
#include <variant>

typedef enum {
  IMMEDIATE,
  DIRECT,
  INDIRECT

} OperandFormat;

typedef enum {
  OP_BR = 0,
  OP_BROPOS = 1,
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

typedef struct vmstate {
  int16_t memory[500];

  int16_t pc, sp, acc, mop, ri, re, r0, r1;

  std::shared_mutex mutex;

  std::atomic<bool> sigRun{false}, sigStep{false}, sigPause{false},
      sigStop{false}, isHalted{false}, hasError{false};

} VMState;

class Operations {
 public:
  // um map de Opcodes direto em ponteiros de funções :O
  // MAIN CHAMA INITIALIZEMAP pra construir o map estático
  // no final isso aqui é só um hashmap elegante
  using OpFunc = void (*)(VMState *);

  static std::map<Opcode, OpFunc> execute;

  static void InitializeMap() {
    execute[OP_BR] = &BR;
    execute[OP_BROPOS] = &BROPOS;
    execute[OP_ADD] = &ADD;
    execute[OP_SUB] = &SUB;
    execute[OP_MULT] = &MULT;
    execute[OP_DIVIDE] = &DIVIDE;
    execute[OP_LOAD] = &LOAD;
    execute[OP_STORE] = &STORE;
    execute[OP_WRITE] = &WRITE;
    execute[OP_BRZERO] = &BRZERO;
    execute[OP_BRNEG] = &BRNEG;
    execute[OP_STOP] = &STOP;
    execute[OP_READ] = &READ;
    execute[OP_COPY] = &COPY;
    execute[OP_CALL] = &CALL;
    execute[OP_RET] = &RET;
    execute[OP_PUSH] = &PUSH;
    execute[OP_POP] = &POP;
  }

  static void BR(VMState *vm) {
    int16_t operand = FetchValue(vm->memory[vm->pc], 0, vm);
    vm->pc = operand;
  }
  static void BROPOS(VMState *vm) {
    int16_t operand = FetchValue(vm->memory[vm->pc], 0, vm);
    if (vm->acc > 0) {
      vm->pc = operand;
    } else {
      vm->pc += 2;
    }
  }
  static void ADD(VMState *vm) {}
  static void SUB(VMState *vm) {
    int16_t operand = FetchValue(vm->memory[vm->pc], 0, vm);
    vm->acc -= vm->memory[operand];
  }
  static void MULT(VMState *vm) {}
  static void DIVIDE(VMState *vm) {}
  static void LOAD(VMState *vm) {
    int16_t operand = FetchValue(vm->memory[vm->pc], 0, vm);
    vm->acc = vm->memory[operand];
  }
  static void STORE(VMState *vm) { vm->memory[vm->pc] = vm->acc; }
  static void WRITE(VMState *vm) {
    int16_t operand = FetchValue(vm->memory[vm->pc], 0, vm);
    std::cout << "Output: " << operand << std::endl;
  }
  static void BRZERO(VMState *vm) {
    int16_t operand = FetchValue(vm->memory[vm->pc], 0, vm);
    if (vm->acc == 0) {
      vm->pc = operand;
    } else {
      vm->pc += 2;
    }
  }
  static void BRNEG(VMState *vm) {
    int16_t operand = FetchValue(vm->memory[vm->pc], 0, vm);
    if (vm->acc < 0) {
      vm->pc = operand;
    } else {
      vm->pc += 2;
    }
  }
  static void STOP(VMState *vm) {}
  static void READ(VMState *vm) {}
  static void COPY(VMState *vm) {}
  static void CALL(VMState *vm) {}
  static void RET(VMState *vm) {}
  static void PUSH(VMState *vm) {}
  static void POP(VMState *vm) {}
};
#endif  // !H_VM

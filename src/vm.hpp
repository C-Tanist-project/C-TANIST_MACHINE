#ifndef H_VM
#define H_VM

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <map>
#include <mutex>
#include <stack>
#include <syncstream>
#include <thread>
#include <variant>

typedef enum { FINISH, RUN, STEP, STOP, CLOSE, NONE } OperationControls;

typedef enum { ACC, R0, R1 } Registers;

typedef enum { IMMEDIATE = 0, DIRECT = 1, INDIRECT = 2 } OperandFormat;

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

typedef struct vmstatestruct {
  int16_t memory[500] = {0};

  int16_t pc, sp, acc, mop, ri, re, r0, r1;

  std::stack<int16_t> updatedMemoryAddresses;

  std::mutex mutex;

  std::atomic<bool> sigRun{false}, sigFinish{false}, sigPause{false},
      sigClose{false}, sigStep{false}, sigStop{false}, isHalted{true},
      isRunning{false}, hasError{false};
} VMState;

void ExecuteStep(VMState &vm);
void VMStateSetup(VMState &vm);

int16_t *FetchRegister(Registers operandIdx, VMState &vm);

int16_t FetchValue(int16_t instruction, unsigned char operandIdx, VMState &vm);
VMState &vmStateSetup();

OperandFormat DecodeOperandFormat(int16_t instruction,
                                  unsigned char operandIdx);
OperationControls PollOperationControls(VMState &vm);

class Operations {
public:
  using OpFunc = void (*)(VMState &);
  friend class VMEngine;

  static inline std::map<Opcode, OpFunc> execute;

  static void InitializeMap() {
    execute[OP_BR] = &BR;
    execute[OP_BRPOS] = &BRPOS;
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

  static void ADD(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.acc += operand;
    std::cout << "Somando " << operand << " ao ACC\n";
  }

  static void MULT(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.acc *= operand;
  }

  static void DIVIDE(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.acc /= operand;
  }

  static void COPY(VMState &vm) {
    int16_t op1 = FetchValue(vm.memory[vm.pc], 0, vm);
    int16_t op2 = FetchValue(vm.memory[vm.pc], 1, vm);

    if (DecodeOperandFormat(vm.memory[vm.pc], 1) == IMMEDIATE) {
      vm.memory[op1] = op2;
    } else {
      vm.memory[op1] = vm.memory[op2];
    }

    {
      std::lock_guard<std::mutex> lock(vm.mutex);
      vm.updatedMemoryAddresses.push(op1);
    }
  }

  static void CALL(VMState &vm) {
    vm.sp += 1;
    vm.memory[vm.sp] = vm.pc;
    {
      std::lock_guard<std::mutex> lock(vm.mutex);
      vm.updatedMemoryAddresses.push(vm.pc);
    }
    vm.pc = vm.memory[vm.pc + 1];
  }

  static void RET(VMState &vm) {
    vm.pc = vm.sp;
    vm.sp -= 1;
  }

  static void STOP(VMState &vm) { vm.isHalted = true; }

  static void READ(VMState &vm) {
    int16_t input;
    std::cin >> input;
  }

  static void PUSH(VMState &vm) {
    int16_t *reg = FetchRegister((Registers)vm.memory[vm.pc + 1], vm);
    vm.sp += 1;
    vm.memory[vm.sp] = *reg;
    {
      std::lock_guard<std::mutex> lock(vm.mutex);
      vm.updatedMemoryAddresses.push(vm.sp);
    }
  }

  static void POP(VMState &vm) {
    int16_t *reg = FetchRegister((Registers)vm.memory[vm.pc + 1], vm);
    *reg = vm.memory[vm.sp];
    {
      std::lock_guard<std::mutex> lock(vm.mutex);
      vm.updatedMemoryAddresses.push(vm.sp);
    }
    vm.sp -= 1;
  }

  static void BR(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.pc = operand;
  }

  static void BRPOS(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    if (vm.acc > 0) {
      vm.pc = operand;
    } else {
      vm.pc += 2;
    }
  }

  static void SUB(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.acc -= vm.memory[operand];
  }

  static void LOAD(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.acc = vm.memory[operand];
  }

  static void STORE(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.memory[operand] = vm.acc;
    {
      std::lock_guard<std::mutex> lock(vm.mutex);
      vm.updatedMemoryAddresses.push(operand);
    }
    std::cout << "Escrevendo ACC no endereço" << operand << "\n";
    std::cout << "Endereço " << operand << " = " << vm.memory[operand] << "\n";
  }

  static void WRITE(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    std::cout << "Output: " << operand << std::endl;
  }

  static void BRZERO(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    if (vm.acc == 0) {
      vm.pc = operand;
    } else {
      vm.pc += 2;
    }
  }

  static void BRNEG(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    if (vm.acc < 0) {
      vm.pc = operand;
    } else {
      vm.pc += 2;
    }
  }
};

class VMEngine {
  std::condition_variable conditionVariable;
  std::mutex mutex;
  std::atomic<bool> hasNewCommand{false};

public:
  void Run(VMState &vm) {
    bool running = false;
    bool stepReady = false;

    while (true) {
      auto control = PollOperationControls(vm);

      if (control != NONE) {
        switch (control) {
        case CLOSE:
          return;
        case FINISH:
          running = true;
          break;
        case STEP:
          stepReady = true;
          break;
        default:
          break;
        }
      }

      if (running && !vm.isHalted) {
        ExecuteStep(vm);
      } else if (stepReady && !vm.isHalted) {
        ExecuteStep(vm);
        stepReady = false;
      } else {
        std::unique_lock<std::mutex> lock(mutex);
        conditionVariable.wait_for(lock, std::chrono::milliseconds(16));
      }
    }
  }

  void NotifyCommand() {
    hasNewCommand = true;
    conditionVariable.notify_one();
  }
};

#endif // !H_VM

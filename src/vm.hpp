#pragma once

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <syncstream>
#include <thread>
#include <variant>

#include "types.hpp"

void ExecuteStep(VMState &vm);
void VMStateSetup(VMState &vm);

int16_t *FetchRegister(Registers operandIdx, VMState &vm);

int16_t FetchValue(int16_t instruction, unsigned char operandIdx, VMState &vm);
VMState &vmStateSetup();

OperandFormat DecodeOperandFormat(int16_t instruction,
                                  unsigned char operandIdx);

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
    vm.acc += vm.memory[operand];
    std::cout << "Somando " << vm.memory[operand] << " ao ACC\n";
  }

  static void MULT(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.acc *= vm.memory[operand];
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
    int16_t op1 = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.sp += 1;
    vm.memory[vm.sp] = vm.pc;
    {
      std::lock_guard<std::mutex> lock(vm.mutex);
      vm.updatedMemoryAddresses.push(vm.sp);
    }
    vm.pc = op1;
  }

  static void RET(VMState &vm) {
    vm.pc = vm.memory[vm.sp] + 1;
    vm.sp -= 1;
  }

  static void STOP(VMState &vm) { vm.isHalted = true; }

  static void READ(VMState &vm) {
    vm.waitingForInput = true;
    while (vm.waitingForInput) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    vm.acc = vm.inputValue;
    std::cout << "Input: " << vm.inputValue << std::endl;  // teste
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
    vm.acc -= operand;  // era vm.acc -= vm.memory[operand];
  }

  static void LOAD(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.acc = operand;  // antes era vm.acc = vm.memory[operand];
  }

  static void STORE(VMState &vm) {
    int16_t operand = FetchValue(vm.memory[vm.pc], 0, vm);
    vm.memory[operand] = vm.acc;
    {
      std::lock_guard<std::mutex> lock(vm.mutex);
      vm.updatedMemoryAddresses.push(operand);
    }
  }

  static void WRITE(VMState &vm) {
    int16_t address = FetchValue(vm.memory[vm.pc], 0, vm);
    int16_t value = vm.memory[address];
    {
      std::lock_guard<std::mutex> lock(vm.consoleMutex);
      vm.consoleMessages.push("Output: " + std::to_string(value));
    }

    std::cout << "Output: " << value << std::endl;
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
  static inline std::condition_variable conditionVariable;
  static inline std::mutex controlMutex;
  static inline std::queue<VMControls> controlQueue;

 public:
  void Run(VMState &vm) {
    bool finishing{false}, stepping{false}, hasInitialCopy{false},
        paused{false};

    static int16_t buffer[500];

    while (true) {
      auto control = GetNextCommand();

      if (control != NONE) {
        switch (control) {
          case CLOSE:
            return;
          case FINISH:
            finishing = true;
          case RUN:
            vm.isRunning.exchange(true);
            paused = false;
            break;
          case STEP:
            stepping = true;
            paused = false;
            break;
          case PAUSE:
            paused = true;
            break;
          case STOP:
            vm.isRunning.exchange(false);
            vm.isHalted.exchange(false);
            hasInitialCopy = false;
            stepping = false;
            finishing = false;
            {
              std::lock_guard<std::mutex> lock(vm.mutex);
              memcpy(vm.memory, buffer, sizeof(buffer));
              vm.updatedMemoryAddresses.push(-1);
            }
            VMStateSetup(vm);
            break;
          case NONE:
            break;
        }
      }

      if ((vm.isRunning || stepping) && !hasInitialCopy) {
        std::lock_guard<std::mutex> lock(vm.mutex);
        memcpy(buffer, vm.memory, sizeof(buffer));
      }

      if (paused) {
        std::unique_lock<std::mutex> lock(controlMutex);
        conditionVariable.wait_for(lock, std::chrono::milliseconds(16));
      } else if (finishing && !vm.isHalted) {
        ExecuteStep(vm);
      } else if (vm.isRunning && !vm.isHalted) {
        ExecuteStep(vm);

        std::unique_lock<std::mutex> lock(controlMutex);
        std::chrono::duration<float> clock(1.0f / vm.clockSpeed);
        conditionVariable.wait_for(lock, clock);

      } else if (stepping && !vm.isHalted) {
        ExecuteStep(vm);
        stepping = false;
      } else {
        std::unique_lock<std::mutex> lock(controlMutex);
        conditionVariable.wait_for(lock, std::chrono::milliseconds(16));
      }
    }
  }

  static void NotifyCommand(VMControls command) {
    {
      std::lock_guard<std::mutex> lock(controlMutex);
      controlQueue.push(command);
      conditionVariable.notify_one();
    }
  }

 private:
  VMControls GetNextCommand() {
    std::lock_guard<std::mutex> lock(controlMutex);
    if (controlQueue.empty()) return NONE;
    auto command = controlQueue.front();
    controlQueue.pop();
    return command;
  }
};

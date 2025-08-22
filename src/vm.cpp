#include "vm.hpp"

void VMStateSetup(VMState &vm) {
  vm.pc = 0;
  vm.sp = 0;
  vm.acc = 0;
  vm.mop = 0;
  vm.ri = 0;
  vm.re = 0;
  vm.r0 = 0;
  vm.r1 = 0;
}

OperandFormat DecodeOperandFormat(int16_t instruction,
                                  unsigned char operandIdx) {
  Opcode opcode = (Opcode)(instruction & 31);

  bool indirectSwitch = instruction & (1 << (5 + operandIdx));
  OperandFormat operandFormat = indirectSwitch ? INDIRECT : DIRECT;

  bool immediateSwitch = instruction & (1 << 7);

  if (!immediateSwitch)
    return operandFormat;

  switch (opcode) {
  case OP_COPY:
    if (operandIdx == 0)
      return operandFormat;
  case OP_ADD:
  case OP_DIVIDE:
  case OP_LOAD:
  case OP_MULT:
  case OP_SUB:
  case OP_WRITE:
    return IMMEDIATE;
    break;
  default:
    return operandFormat;
  }
}

int16_t *FetchRegister(Registers operandIdx, VMState &vm) {
  switch (operandIdx) {
  // case ACC:
  //   return &(vm.acc);
  //   break;
  case R0:
    return &(vm.r0);
  case R1:
    return &(vm.r1);
  default:
    return &(vm.acc);
  }
}

int16_t FetchValue(int16_t instruction, unsigned char operandIdx, VMState &vm) {
  OperandFormat operandFormat = DecodeOperandFormat(instruction, operandIdx);
  int16_t rawOperandAddress = vm.pc + operandIdx + 1;
  int16_t rawOperandValue = vm.memory[rawOperandAddress];

  std::cout << "RawOperandValue = " << rawOperandValue << "\n";

  switch (operandFormat) {
  case IMMEDIATE:
    return rawOperandValue;
  case DIRECT:
    return vm.memory[rawOperandValue];
  case INDIRECT:
    return vm.memory[vm.memory[rawOperandValue]];
  default:
    return rawOperandValue;
  }
}

void ExecuteStep(VMState &vm) {
  unsigned char offset = 0;

  int16_t instruction = vm.memory[vm.pc];

  Opcode opcode = (Opcode)((instruction & 31) % 19);

  switch (opcode) {
  case OP_COPY:
    offset = 3;
    break;
  case OP_BR:
  case OP_BRNEG:
  case OP_BRZERO:
    offset = 0;
    break;
  case OP_RET:
  case OP_STOP:
    offset = 1;
    break;
  case OP_PUSH:
    if (vm.sp == vm.memory[1]) {
      vm.pc = 0;
      {
        std::lock_guard<std::mutex> lock(vm.consoleMutex);
        vm.consoleMessages.push("EXCEPTION: Stack Overflow.");
      }
      vm.isHalted = true;
      return;
    }
  case OP_POP:
    if (vm.sp == 2) {
      vm.pc = 0;
      {
        std::lock_guard<std::mutex> lock(vm.consoleMutex);
        vm.consoleMessages.push("EXCEPTION: Stack Underflow.");
      }
      vm.isHalted = true;
      return;
    }
  default:
    offset = 2;
  }

  Operations::execute[opcode](vm);

  vm.pc += offset;
}

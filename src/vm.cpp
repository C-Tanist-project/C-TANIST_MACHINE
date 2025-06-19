#include "vm.hpp"

std::map<Opcode, Operations::OpFunc> Operations::execute;

OperandFormat DecodeOperandFormat(int16_t instruction,
                                  unsigned char operandIdx) {
    Opcode opcode = (Opcode)(instruction & 31);

    bool indirectSwitch = instruction & (1 << (5 + operandIdx));
    OperandFormat operandFormat = indirectSwitch ? INDIRECT : DIRECT;

    bool immediateSwitch = instruction & (1 << 7);
    if (immediateSwitch) return operandFormat;

    switch (opcode) {
        case OP_COPY:
            if (operandIdx == 0) return operandFormat;
        case OP_ADD:
        case OP_DIVIDE:
        case OP_LOAD:
        case OP_MULT:
        case OP_SUB:
        case OP_WRITE:
            return IMMEDIATE;
        default:
            return operandFormat;
    }
}

int16_t FetchValue(int16_t instruction, unsigned char operandIdx, VMState *vm) {
    OperandFormat operandFormat = DecodeOperandFormat(instruction, operandIdx);

    switch (operandFormat) {
        case IMMEDIATE:
            return vm->memory[vm->pc + operandIdx + 1];
        case INDIRECT:
            return vm->memory[vm->memory[vm->pc + operandIdx + 1]];
        case DIRECT:
            return vm->memory[vm->pc + operandIdx + 1];
    }
}

void ExecuteStep(VMState *vm) {
    unsigned char offset = 0;
    int16_t instruction = vm->memory[vm->pc];

    Opcode opcode = (Opcode)((instruction & 31) % 19);

    switch (opcode) {
        case OP_COPY:
            offset = 3;
            break;
        case OP_RET:
        case OP_STOP:
            offset = 1;
            break;
        default:
            offset = 2;
    }

    Operations::execute[opcode](vm);

    vm->pc += offset;
}

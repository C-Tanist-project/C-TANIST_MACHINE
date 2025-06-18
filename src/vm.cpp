#include "vm.hpp"

void ExecuteStep(VMState *vmState) {
    unsigned char offset = 0;
    int16_t instruction = vmState->memory[vmState->pc];

    Opcode opcode = (Opcode)(instruction & 31);

    OperatorFormat operator1Format =
        (instruction) & (1 << 5) ? INDIRECT : DIRECT;
    OperatorFormat operator2Format =
        (instruction) & (1 << 6) ? INDIRECT : DIRECT;

    if ((instruction) & (1 << 7)) {
        operator1Format = IMMEDIATE;
    }

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

    vmState->pc += offset;
}

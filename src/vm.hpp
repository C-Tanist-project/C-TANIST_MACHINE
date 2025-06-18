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

} OperatorFormat;

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
    // MAIN CHAMA ESSA FUNÇÂO pra construir o map estático
    using OpFunc = void (*)(VMState *, int16_t *);

    static std::map<Opcode, OpFunc> executeFunction;

    static void InitializeMap() {
        executeFunction[OP_ADD] = &ADD;
        executeFunction[OP_SUB] = &SUB;
        executeFunction[OP_MULT] = &MULT;
        executeFunction[OP_DIVIDE] = &DIVIDE;
    }

    static void ADD(VMState *vmState, int16_t *operands) {}
    static void SUB(VMState *vmState, int16_t *operands) {}
    static void MULT(VMState *vmState, int16_t *operands) {}
    static void DIVIDE(VMState *vmState, int16_t *operands) {}
};
#endif  // !H_VM

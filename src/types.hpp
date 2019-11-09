#ifndef FILE_SEEN_TYPES
#define FILE_SEEN_TYPES

#include <cstdint>
#include <variant>

const static size_t RegisterSize = 1;
const static size_t ImmediateSize = 4;
const static size_t InstructionSize = 1;

// Same bytecode
enum class Register {
    Alpha=0,
    Beta=1
};
using Immediate = uint32_t;

// nand %alpha %beta
struct NandRR {
    const static size_t size = InstructionSize + RegisterSize + RegisterSize;

    Register to;
    Register from;
    explicit NandRR (Register t_to, Register t_from) : to(t_to), from(t_from) {}
};
// nand %alpha 0x5
struct NandRI {
    const static size_t size = InstructionSize + RegisterSize + ImmediateSize;

    Register to;
    Immediate num;
    explicit NandRI (Register t_to, Immediate t_num) : to(t_to), num(t_num) {}
};

struct ResetR {
    const static size_t size = InstructionSize + RegisterSize;

    Register victim;
    explicit ResetR (Register t_victim) : victim(t_victim) {}
};


using Instruction = std::variant<NandRR, NandRI, ResetR>;
enum class InstructionCode {
    NandRI = 0x05,
    NandRR = 0x06,
    ResetR = 0x07
};

#endif
#ifndef FILE_SEEN_TYPES
#define FILE_SEEN_TYPES

#include <cstdint>
#include <variant>

// Same bytecode
enum class Register {
    Alpha=0,
    Beta=1
};
using Immediate = uint32_t;

// nand %alpha %beta
struct NandRR {
    Register to;
    Register from;
    explicit NandRR (Register t_to, Register t_from) : to(t_to), from(t_from) {}
};
// nand %alpha 0x5
struct NandRI {
    Register to;
    Immediate num;
    explicit NandRI (Register t_to, Immediate t_num) : to(t_to), num(t_num) {}
};

struct ResetR {
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
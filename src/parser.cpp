#include <iostream>
#include <string>
#include <vector>
#include "types.hpp"

class Parser {
    public:

    std::string code;

    std::vector<Instruction> instructions;

    Parser (std::string t_code) : code(t_code) {}

    bool isValidIndex (size_t i) {
        return i < code.size();
    }

    void parse () {
        for (size_t i = 0; isValidIndex(i);) {
            consumeWhitespace(i);

            // Comment, consume until end of line
            if (code[i] == ';') {
                for (; isValidIndex(i);) {
                    if (code[i] == '\n') {
                        i++;
                        break;
                    }
                    i++;
                }

                continue;
            }

            if (!isTextCharacter(code[i])) {
                throw std::runtime_error("Unexpected character '" + std::to_string(code[i]) + "' at position " + std::to_string(i));
            }

            size_t instruction_name_size = 0;
            const size_t instruction_name_index = i;

            for (; isValidIndex(i) && isTextCharacter(code[i]); i++) {
                instruction_name_size++;
            }

            std::string instruction_name = code.substr(instruction_name_index, instruction_name_size);

            if (instruction_name == "nand") {
                Register to;
                if (!parseRegister(i, to)) {
                    throw std::runtime_error("Expected first argument to `nand` instruction to be a register.");
                }

                Register from;
                if (!parseRegister(i, from)) {
                    Immediate num;
                    if (!parseNumber(i, num)) {
                        throw std::runtime_error("Unknown second parameter to `nand` instruction. Expected register|immediate");
                    } else {
                        instructions.push_back(NandRI(to, num));
                    }
                } else {
                    instructions.push_back(NandRR(to, from));
                }
            } else if (instruction_name == "reset") {
                Register victim;
                if (!parseRegister(i, victim)) {
                    throw std::runtime_error("Unknown first parameter to `reset` instruction. Expected register.");
                } else {
                    instructions.push_back(ResetR(victim));
                }
            } else {
                throw std::runtime_error("Unknown instruction name: '" + instruction_name + "'");
            }

            if (!isValidIndex(i)) {
                break;
            }

            // I HATE NEWLINES
            if (code[i] == '\n' || code[i] == '\r') {
                i++;
                if (isValidIndex(i)) {
                    if (code[i] == '\n' || code[i] == '\r') {
                        i++;
                    }
                }
            }
        }
    }

    bool parseNumber (size_t& i, Immediate& num) {
        size_t initial_i = i;

        consumeWhitespace(i);

        if (!isValidIndex(i)) {
            std::cout << "Number parsing went past end, stopping..\n";
            i = initial_i;
            return false;
        }

        if (!isdigit(code[i])) {
            std::cout << "Number parsing found character that wasn't digit at start.. " + std::to_string(code[i]) + "\n";
            i = initial_i;
            return false;
        }

        size_t number_size = 0;
        const size_t number_index = i;

        for (; isValidIndex(i) && isdigit(code[i]); i++) {
            number_size++;
        }

        std::string number_text = code.substr(number_index, number_size);
        Immediate result = std::stoi(number_text);

        num = result;

        return true;
    }

    // `ret` is set to a value if the return value is true
    bool parseRegister (size_t& i, Register& reg) {
        size_t initial_i = i;

        // Consume whitespace
        consumeWhitespace(i);

        if (!isValidIndex(i)) {
            i = initial_i;
            return false;
        }

        if (code[i] != '%') {
            i = initial_i;
            return false;
        }

        i++;

        size_t register_size = 0;
        const size_t register_index = i;

        if (!isValidIndex(i)) {
            i = initial_i;
            return false;
        }
        if (!isTextCharacter(code[i])) {
            i = initial_i;
            return false;
        }

        for (; isValidIndex(i) && isTextCharacter(code[i]); i++) {
            register_size++;
        }

        std::string register_name = code.substr(register_index, register_size);

        if (register_name == "alpha") {
            reg = Register::Alpha;
            return true;
        } else if (register_name == "beta") {
            reg = Register::Beta;
            return true;
        } else {
            i = initial_i;
            return false;
        }
    }

    void consumeWhitespace (size_t& i) {
        for (; isValidIndex(i) && isWhitespace(code[i]); i++) {}
    }

    bool isTextCharacter (char c) {
        return islower(c) || isupper(c);
    }

    bool isWhitespace (char c) {
        return c == ' ' || c == '\n' || c == '\r' || c == '\f' || c == '\t';
    }

    std::vector<uint8_t> bytecode () {
        std::vector<uint8_t> values;

        for (Instruction& ins : instructions) {
            // 5 is arbitrary starting point for instructions, so we can put some before or something idk
            if (std::holds_alternative<NandRI>(ins)) {
                NandRI n = std::get<NandRI>(ins);
                values.push_back(0x05); // Nand reg immediate
                values.push_back(bytecodeRegister(n.to));
                bytecodeImmediate(values, n.num);
            } else if (std::holds_alternative<NandRR>(ins)) {
                NandRR n = std::get<NandRR>(ins);
                values.push_back(0x06); // nand reg reg
                values.push_back(bytecodeRegister(n.to));
                values.push_back(bytecodeRegister(n.from));
            } else if (std::holds_alternative<ResetR>(ins)) {
                ResetR r = std::get<ResetR>(ins);
                values.push_back(0x07); // reset reg
                values.push_back(bytecodeRegister(r.victim));
            } else {
                throw std::runtime_error("Can not generate bytecode for unknown instruction!");
            }
        }

        return values;
    }

    uint8_t bytecodeRegister (Register r) {
        // 3 is arbitrary start
        if (r == Register::Alpha) {
            return 3;
        } else if (r == Register::Beta) {
            return 4;
        } else {
            return 0; // invalid register, THis should throw an error, or perhaps we could have a null-register?
        }
    }

    void bytecodeImmediate (std::vector<uint8_t>& values, Immediate value) {
        // assumes 32-bit integer
        values.push_back((value & 0xFF000000) >> 24);
        values.push_back((value & 0x00FF0000) >> 16);
        values.push_back((value & 0x0000FF00) >> 8);
        values.push_back(value & 0x000000FF);
    }

    void debug () {
        for (Instruction& ins : instructions) {
            if (std::holds_alternative<NandRI>(ins)) {
                NandRI n = std::get<NandRI>(ins);
                std::cout << "NandRI: ";
                printRegister(n.to);
                std::cout << " " << n.num << "\n";
            } else if (std::holds_alternative<NandRR>(ins)) {
                NandRR n = std::get<NandRR>(ins);
                std::cout << "NandRR: ";
                printRegister(n.to);
                std::cout << " ";
                printRegister(n.from);
                std::cout << "\n";
            } else if (std::holds_alternative<ResetR>(ins)) {
                ResetR r = std::get<ResetR>(ins);
                std::cout << "ResetR: ";
                printRegister(r.victim);
                std::cout << "\n";
            } else {
                std::cout << "Unhandled instruction\n";
            }
        }
    }

    void printRegister (Register r) {
        if (r == Register::Alpha) {
            std::cout << "Alpha";
        } else if (r == Register::Beta) {
            std::cout << "Beta";
        } else {
            std::cout << "UKNOWN";
        }
    }
};

int main () {
    Parser p = Parser(
        "nand %alpha 45\n"
        "reset %beta; abc\n"
        " ;  test;a"
    );
    p.parse();
    p.debug();
    std::cout << "Bytecode: \n";
    auto x = p.bytecode();

    std::cout << std::hex;
    for (uint8_t v : x) {
        std::cout << unsigned(v) << " ";
    }
}
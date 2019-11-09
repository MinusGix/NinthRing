#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include "types.hpp"

bool isHexadecimalDigit (char c) {
    return isdigit(c) || c == 'a' || c == 'A' || c == 'b' || c == 'B' || c == 'c' || c == 'C' ||
        c == 'd' || c == 'D' || c == 'e' || c == 'E' || c == 'f' || c == 'F';
}

bool isBinaryDigit (char c) {
    return c == '0' || c == '1';
}

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
            i = initial_i;
            return false;
        }

        if (code[i] == '0' && isValidIndex(i + 1)) {
            if (code[i + 1] == 'x') {
                i += 2; // skip 0x
                if (!parseHexadecimal(i, num)) {
                    i = initial_i;
                    return false;
                } else {
                    return true;
                }
            } else if (code[i + 1] == 'b') {
                i += 2; // skip 0b
                if (!parseBinary(i, num)) {
                    i = initial_i;
                    return false;
                } else {
                    return true;
                }
            } else if (!isdigit(code[i] + 1)) { // if it's a digit, lets just ignore it and do normal number parsing
                throw std::runtime_error("Unknown modifier to number at position " + std::to_string(i + 1));
            }
        }

        if (!isdigit(code[i])) {
            i = initial_i;
            return false;
        }

        size_t number_size = 0;
        const size_t number_index = i;

        for (; isValidIndex(i) && isdigit(code[i]); i++) {
            number_size++;
        }

        std::string number_text = code.substr(number_index, number_size);

        // TODO: Hopefully this cast is fine..
        Immediate result = static_cast<Immediate>(std::stoul(number_text));

        num = result;

        return true;
    }
    // It'd be nice to not have repeats

    bool parseHexadecimal (size_t& i, Immediate& num) {
        if (!isHexadecimalDigit(code[i])) {
            return false;
        }

        size_t number_size = 0;
        const size_t number_index = i;

        for (; isValidIndex(i) && isHexadecimalDigit(code[i]); i++) {
            number_size++;
        }

        std::string number_text = code.substr(number_index, number_size);
        // TODO: Hopefully this cast is fine..
        Immediate result = static_cast<Immediate>(std::stoul(number_text, nullptr, 16));

        num = result;

        return true;
    }

    bool parseBinary (size_t& i, Immediate& num) {
        if (!isBinaryDigit(code[i])) {
            return false;
        }

        size_t number_size = 0;
        const size_t number_index = i;

        for (; isValidIndex(i) && isBinaryDigit(code[i]); i++) {
            number_size++;
        }

        std::string number_text = code.substr(number_index, number_size);
        // TODO: Hopefully this cast is fine..
        Immediate result = static_cast<Immediate>(std::stoul(number_text, nullptr, 2));

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
                values.push_back(static_cast<uint8_t>(InstructionCode::NandRI)); // Nand reg immediate
                values.push_back(bytecodeRegister(n.to));
                bytecodeImmediate(values, n.num);
            } else if (std::holds_alternative<NandRR>(ins)) {
                NandRR n = std::get<NandRR>(ins);
                values.push_back(static_cast<uint8_t>(InstructionCode::NandRR)); // nand reg reg
                values.push_back(bytecodeRegister(n.to));
                values.push_back(bytecodeRegister(n.from));
            } else if (std::holds_alternative<ResetR>(ins)) {
                ResetR r = std::get<ResetR>(ins);
                values.push_back(static_cast<uint8_t>(InstructionCode::ResetR)); // reset reg
                values.push_back(bytecodeRegister(r.victim));
            } else {
                throw std::runtime_error("Can not generate bytecode for unknown instruction!");
            }
        }

        return values;
    }

    uint8_t bytecodeRegister (Register r) {
        return static_cast<uint8_t>(r);
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

int main (int argc, char** argv) {
    if (argc == 1) {
        std::cout << "Parser/Bytecodifier\nparser [input-file] [output-file]\nEx: parser examples/basic.nr build/basic.nv\n";
        return 0;
    } else if (argc == 2) {
        std::cout << "Requires output file\n";
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = argv[2];

    if (!std::filesystem::exists(input_filename)) {
        std::cout << "Cannot find file: " << input_filename << "\n";
        return 1;
    }

    std::ifstream input_file(input_filename);

    // Read entire file
    std::string input_text(
        (std::istreambuf_iterator<char>(input_file)),
        (std::istreambuf_iterator<char>())
    );

    input_file.close();

    Parser p = Parser(input_text);
    p.parse();
    p.debug();
    std::cout << "Bytecode: \n";
    auto x = p.bytecode();

    std::cout << std::hex;
    for (uint8_t v : x) {
        std::cout << unsigned(v) << " ";
    }

    std::ofstream output_file(output_filename, std::ios::out | std::ios::binary);
    output_file.write(reinterpret_cast<char*>(x.data()), x.size());
}
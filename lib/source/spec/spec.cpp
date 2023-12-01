#include <disasm/spec/spec.hpp>

#include <format>

namespace disasm::spec {

    namespace {

        std::pair<std::string, size_t> formatOpcode(const std::vector<Opcode> &opcodes, std::span<const u8> bytes, bool allowMultiple = false) {
            std::string result;
            size_t advance = 0;
            for (const Opcode &opcode : opcodes) {
                if (auto formattedValue = opcode.format(bytes.subspan(advance)); formattedValue.has_value()) {
                    result += formattedValue.value();
                    result.append(" ");

                    advance += opcode.getSize();

                    if (!allowMultiple)
                        break;
                }
            }

            return { result, advance };
        }

    }

    std::vector<std::string> Spec::disassemble(const std::span<const u8> bytes) const {
        std::vector<std::string> disassembly;

        size_t offset = 0x00;
        std::string result;
        while (offset < bytes.size()) {
            {
                auto [formattedValue, advance] = formatOpcode(getPrefixes(), bytes.subspan(offset), true);
                result += formattedValue;
                offset += advance;
            }

            {
                auto [formattedValue, advance] = formatOpcode(getOpcodes(), bytes.subspan(offset));
                result += formattedValue;
                offset += 1;

                if (advance == 0) {
                    result.clear();
                    disassembly.emplace_back("???");
                    offset += 1;
                } else {
                    disassembly.emplace_back(std::move(result));
                }
            }

        }

        return disassembly;
    }

}


#include <disasm/spec/spec.hpp>

#include <format>

namespace disasm::spec {

    std::vector<std::string> Spec::disassemble(const std::span<const u8> bytes) const {
        std::vector<std::string> disassembly;

        size_t offset = 0x00;
        std::string result;
        while (offset < bytes.size()) {
            bool found = false;

            for (const Opcode &prefix : getPrefixes()) {
                if (auto formattedValue = prefix.format(bytes.subspan(offset)); formattedValue.has_value()) {
                    result += formattedValue.value();
                    result.append(" ");

                    offset += prefix.getSize();
                    found = true;
                }
            }

            for (const Opcode &opcode : getOpcodes()) {
                if (auto formattedValue = opcode.format(bytes.subspan(offset)); formattedValue.has_value()) {
                    result += formattedValue.value();
                    offset += opcode.getSize();
                    found = true;
                    break;
                }
            }

            if (!found) {
                result.clear();
                disassembly.emplace_back("???");
                offset += 1;
            } else {
                disassembly.emplace_back(std::move(result));
            }
        }

        return disassembly;
    }



}


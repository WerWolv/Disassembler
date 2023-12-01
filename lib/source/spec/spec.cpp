#include <disasm/spec/spec.hpp>


namespace disasm::spec {

    std::vector<std::string> Spec::disassemble(const std::span<const u8> bytes) const {
        std::vector<std::string> result;

        size_t offset = 0x00;
        while (offset < bytes.size()) {
            bool found = false;
            for (const Opcode &opcode : getOpcodes()) {
                if (auto formattedValue = opcode.format(bytes.subspan(offset)); formattedValue.has_value()) {
                    result.emplace_back(std::move(formattedValue.value()));
                    offset += opcode.getSize();
                    found = true;
                    break;
                }
            }

            if (!found) {
                result.emplace_back("db " + std::to_string(bytes[offset]));
                offset += 1;
            }
        }

        return result;
    }



}


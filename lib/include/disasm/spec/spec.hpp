#pragma once

#include <disasm/spec/opcode.hpp>

#include <vector>

namespace disasm::spec {

    class Spec {
    public:
        Spec(std::string name, std::vector<Opcode> opcodes)
            : m_name(std::move(name)),
              m_opcodes(std::move(opcodes)) { }

        [[nodiscard]] const std::string& getName() const noexcept { return m_name; }
        [[nodiscard]] const std::vector<Opcode>& getOpcodes() const noexcept { return m_opcodes; }

        [[nodiscard]] std::vector<std::string> disassemble(std::span<const u8> bytes) const;

    private:
        std::string m_name;
        std::vector<Opcode> m_opcodes;
    };

}
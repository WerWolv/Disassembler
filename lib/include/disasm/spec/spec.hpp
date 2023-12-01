#pragma once

#include <disasm/spec/opcode.hpp>

#include <vector>

namespace disasm::spec {

    class Spec {
    public:
        Spec(std::string name, std::vector<Opcode> opcodes, std::vector<Opcode> prefixes)
            : m_name(std::move(name)),
              m_opcodes(std::move(opcodes)),
              m_prefixes(std::move(prefixes)) { }

        [[nodiscard]] const std::string& getName() const noexcept { return m_name; }
        [[nodiscard]] const std::vector<Opcode>& getOpcodes() const noexcept { return m_opcodes; }
        [[nodiscard]] const std::vector<Opcode>& getPrefixes() const noexcept { return m_prefixes; }

        [[nodiscard]] std::vector<std::string> disassemble(std::span<const u8> bytes) const;

    private:
        std::string m_name;
        std::vector<Opcode> m_opcodes, m_prefixes;
    };

}
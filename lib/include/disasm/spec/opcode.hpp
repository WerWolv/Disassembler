#pragma once

#include <disasm/types.hpp>
#include <disasm/spec/bit_pattern.hpp>

#include <string>
#include <optional>

namespace disasm::spec {

    class Opcode {
    public:
        Opcode(std::string mask, std::string format);

        [[nodiscard]] std::optional<std::string> format(std::span<const u8> bytes) const;
        [[nodiscard]] u32 getSize() const noexcept { return m_bitPattern.getSize(); }

    private:
        [[nodiscard]] std::optional<std::string> evaluateFormatSpecifier(std::string_view formatSpecifier, std::span<const u8> bytes) const;

    private:
        std::string m_format;
        BitPattern m_bitPattern;
    };

}
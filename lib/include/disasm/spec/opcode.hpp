#pragma once

#include <disasm/types.hpp>
#include <disasm/spec/bit_pattern.hpp>

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace disasm::spec {

    class Opcode {
    public:
        Opcode(std::string mnemonic, std::string mask, std::string format, nlohmann::json metadata);

        [[nodiscard]] const std::string& getMnemonic() const { return this->m_mnemonic; }
        [[nodiscard]] std::optional<std::string> format(u64 address, std::span<const u8> bytes) const;
        [[nodiscard]] u32 getSize() const noexcept { return m_bitPattern.getSize(); }

        [[nodiscard]] const nlohmann::json& getMetadata() const noexcept { return m_metadata; }

        [[nodiscard]] std::optional<i128> evaluateExpression(const std::string &expression, u64 address, std::span<const u8> bytes) const;

    private:
        [[nodiscard]] std::optional<std::string> evaluateFormatSpecifier(std::string_view formatSpecifier, u64 address, std::span<const u8> bytes) const;

    private:
        std::string m_mnemonic;
        std::string m_format;
        nlohmann::json m_metadata;

        BitPattern  m_bitPattern;
    };

}

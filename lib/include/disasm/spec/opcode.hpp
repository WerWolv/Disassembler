#pragma once

#include <disasm/types.hpp>
#include <disasm/spec/bit_pattern.hpp>

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace disasm::spec {

    class Opcode {
    public:
        Opcode(std::string mnemonic, std::string mask, std::string format, std::string sizeofSpec, std::string ifSpec, nlohmann::json metadata);

        [[nodiscard]] const std::string& getMnemonic() const { return this->m_mnemonic; }
        [[nodiscard]] std::optional<std::string> format(u64 address, std::span<const u8> bytes, const std::map<std::string, std::vector<std::string>> &tables, const std::map<std::string, i64> &variables) const;
        [[nodiscard]] u32 getSize(u64 address, std::span<const u8> bytes, const std::map<std::string, i64> &variables) const noexcept;

        [[nodiscard]] const nlohmann::json& getMetadata() const noexcept { return m_metadata; }

        [[nodiscard]] std::optional<i64> evaluateExpression(const std::string &expression, u64 address, std::span<const u8> bytes, const std::map<std::string, i64> &variables) const;

    private:
        [[nodiscard]] std::optional<std::string> evaluateFormatSpecifier(std::string_view formatSpecifier, const std::map<std::string, std::vector<std::string>> &tables, const std::map<std::string, i64> &variables, u64 address, std::span<const u8> bytes) const;
        [[nodiscard]] std::optional<std::string> evaluateLUTFormatSpecifier(std::string_view formatSpecifier, const std::map<std::string, std::vector<std::string>> &tables, std::span<const u8> bytes) const;

    private:
        std::string m_mnemonic;
        std::string m_format;
        std::string m_sizeof;
        std::string m_ifSpec;
        nlohmann::json m_metadata;

        BitPattern  m_bitPattern;
    };

}

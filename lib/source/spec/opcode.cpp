#include <format>
#include <algorithm>
#include <ranges>
#include <disasm/spec/opcode.hpp>

#include <wolv/utils/string.hpp>
#include <wolv/math_eval/math_evaluator.hpp>

#include <fmt/core.h>

namespace disasm::spec {

    Opcode::Opcode(std::string mnemonic, std::string mask, std::string format, nlohmann::json metadata)
        : m_mnemonic(std::move(mnemonic)),
          m_format(std::move(format)),
          m_metadata(std::move(metadata)),
          m_bitPattern(std::move(mask)) { }

    std::optional<i64> Opcode::evaluateExpression(const std::string &expression, u64 address, const std::span<const u8> bytes) const {
        wolv::math_eval::MathEvaluator<i64> mathEvaluator;

        mathEvaluator.setVariable("offset", address);
        for (const auto &placeholder : this->m_bitPattern.getPlaceholders()) {
            mathEvaluator.setVariable(std::string(1, placeholder), this->m_bitPattern.valueOf(placeholder, bytes));
        }

        const auto result = mathEvaluator.evaluate(expression);

        return result;
    }

    std::optional<std::string> Opcode::evaluateFormatSpecifier(std::string_view formatSpecifier, u64 address, const std::span<const u8> bytes) const {
        std::string formatString;
        i64 value = 0;

        const auto delimiterCount = std::ranges::count(formatSpecifier, ':');
        if (delimiterCount == 1) {
            const auto parts = wolv::util::splitString(std::string(formatSpecifier), ":");
            if (parts.size() != 2)
                return std::nullopt;

            const auto result = this->evaluateExpression(parts[0], address, bytes);
            if (!result.has_value())
                return std::nullopt;

            formatString = fmt::format("{{0:{}}}", parts[1]);
            value = result.value();
        } else if (delimiterCount > 1) {
            return std::nullopt;
        } else {
            formatString = "{}";
            value = m_bitPattern.valueOf(formatSpecifier[0], bytes);
        }

        return fmt::vformat(formatString, fmt::make_format_args(value));
    }

    std::optional<std::string> Opcode::format(u64 address, const std::span<const u8> bytes) const {
        if (!this->m_bitPattern.matches(bytes))
            return std::nullopt;

        std::string result;
        for (size_t i = 0; i < this->m_format.size(); i++) {
            if (this->m_format[i] == '{') {
                i += 1;
                if (i >= this->m_format.size())
                    return std::nullopt;

                if (this->m_format[i] == '{') {
                    result += '{';
                    continue;
                }

                std::string formatSpecifier;
                while (this->m_format[i] != '}') {
                    formatSpecifier += this->m_format[i];
                    i += 1;
                    if (i >= this->m_format.size())
                        return std::nullopt;
                }

                if (auto formattedValue = evaluateFormatSpecifier(formatSpecifier, address, bytes); formattedValue.has_value()) {
                    result += formattedValue.value();
                } else {
                    return std::nullopt;
                }
            } else {
                result += this->m_format[i];
            }
        }

        return result;
    }

}


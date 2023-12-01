#include <format>
#include <algorithm>
#include <ranges>
#include <disasm/spec/opcode.hpp>

#include <wolv/utils/string.hpp>
#include <wolv/math_eval/math_evaluator.hpp>

namespace disasm::spec {

    Opcode::Opcode(std::string mask, std::string format)
        : m_format(std::move(format)),
          m_bitPattern(std::move(mask)) { }

    std::optional<std::string> Opcode::evaluateFormatSpecifier(std::string_view formatSpecifier, const std::span<const u8> bytes) const {
        std::string formatString;
        i128 value = 0;

        const auto delimiterCount = std::ranges::count(formatSpecifier, ':');
        if (delimiterCount == 1) {
            const auto parts = wolv::util::splitString(std::string(formatSpecifier), ":");
            if (parts.size() != 2)
                return std::nullopt;

            wolv::math_eval::MathEvaluator<i128> mathEvaluator;

            for (const auto &placeholder : this->m_bitPattern.getPlaceholders()) {
                mathEvaluator.setVariable(std::string(1, placeholder), this->m_bitPattern.valueOf(placeholder, bytes));
            }

            const auto result = mathEvaluator.evaluate(parts[0]);
            if (!result.has_value())
                return std::nullopt;

            formatString = std::format("{{0:{}}}", parts[1]);
            value = result.value();
        } else if (delimiterCount > 1) {
            return std::nullopt;
        } else {
            formatString = "{}";
            value = m_bitPattern.valueOf(formatSpecifier[0], bytes);
        }

        return std::vformat(formatString, std::make_format_args(value));
    }


    std::optional<std::string> Opcode::format(const std::span<const u8> bytes) const {
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

                if (auto formattedValue = evaluateFormatSpecifier(formatSpecifier, bytes); formattedValue.has_value()) {
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


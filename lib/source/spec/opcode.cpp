#include <algorithm>
#include <ranges>
#include <disasm/spec/opcode.hpp>

#include <wolv/utils/string.hpp>
#include <wolv/math_eval/math_evaluator.hpp>

#include <fmt/format.h>

namespace disasm::spec {

    Opcode::Opcode(std::string mnemonic, std::string mask, std::string format, std::string sizeofSpec, std::string ifSpec, nlohmann::json metadata)
        : m_mnemonic(std::move(mnemonic)),
          m_format(std::move(format)),
          m_sizeof(std::move(sizeofSpec)),
          m_ifSpec(std::move(ifSpec)),
          m_metadata(std::move(metadata)),
          m_bitPattern(std::move(mask)) { }

    namespace {

        template<std::endian Endian>
        std::optional<i64> changeEndiannessFunction(const std::vector<i64> &args) {
            const auto value = args[0];
            auto size = args[1];

            if constexpr (Endian == std::endian::native) {
                return value;
            } else {
                size = std::min<size_t>(size, sizeof(i64));

                std::array<uint8_t, sizeof(i64)> data = { };
                std::memcpy(data.data(), &value, size);

                for (uint32_t i = 0; i < size / 2; i++) {
                    std::swap(data[i], data[size - 1 - i]);
                }

                i64 result = { };
                std::memcpy(&result, data.data(), size);

                return result;
            }
        }

        void registerFunctions(wolv::math_eval::MathEvaluator<i64> &mathEvaluator) {
            mathEvaluator.setFunction("le", changeEndiannessFunction<std::endian::little>, 2, 2);
            mathEvaluator.setFunction("be", changeEndiannessFunction<std::endian::big>, 2, 2);
            mathEvaluator.setFunction("asEndian", [](std::vector<i64> args) {
                const auto func =
                    args[0] == 0 ?
                    changeEndiannessFunction<std::endian::little> :
                    changeEndiannessFunction<std::endian::big>;

                return func({ args[1], args[2] });
            }, 3, 3);
        }

    }

    u32 Opcode::getSize(const u64 address, const std::span<const u8> bytes, const std::map<std::string, i64> &variables) const noexcept {
        const auto size = m_bitPattern.getSize();
        if (m_sizeof.empty()) {
            return size;
        }

        return size + evaluateExpression(m_sizeof, address, bytes, variables).value_or(0);
    }

    std::optional<i64> Opcode::evaluateExpression(const std::string &expression, u64 address, const std::span<const u8> bytes, const std::map<std::string, i64> &variables) const {
        wolv::math_eval::MathEvaluator<i64> mathEvaluator;

        mathEvaluator.setVariable("offset", address);
        for (const auto &placeholder : this->m_bitPattern.getPlaceholders()) {
            mathEvaluator.setVariable(std::string(1, placeholder), this->m_bitPattern.valueOf(placeholder, bytes));
        }
        for (const auto &variable : variables) {
            mathEvaluator.setVariable(variable.first, variable.second);
        }
        registerFunctions(mathEvaluator);


        const auto result = mathEvaluator.evaluate(expression);

        return result;
    }

    std::optional<std::string> Opcode::evaluateLUTFormatSpecifier(std::string_view formatSpecifier, const std::map<std::string, std::vector<std::string> > &tables, std::span<const u8> bytes) const {
        std::string_view tableName;
        std::string_view specifier;

        auto specifierStart = formatSpecifier.find_first_of('[');
        auto specifierEnd = formatSpecifier.find_first_of(']');
        if (specifierStart == std::string_view::npos || specifierEnd == std::string_view::npos)
            return std::nullopt;
        if (specifierStart >= specifierEnd)
            return std::nullopt;

        tableName = formatSpecifier.substr(0, specifierStart);
        specifier = formatSpecifier.substr(specifierStart + 1, specifierEnd - specifierStart - 1);
        if (specifier.size() != 1 || tableName.empty())
            return std::nullopt;

        const auto value = m_bitPattern.valueOf(specifier[0], bytes);

        const auto tableIt = tables.find(std::string(tableName));
        if (tableIt == tables.end())
            return std::nullopt;

        if (value >= tableIt->second.size())
            return std::nullopt;

        return tableIt->second[value];
    }

    std::optional<std::string> Opcode::evaluateFormatSpecifier(std::string_view formatSpecifier, const std::map<std::string, std::vector<std::string>> &tables, const std::map<std::string, i64> &variables, u64 address, const std::span<const u8> bytes) const {
        std::string formatString;
        i64 value = 0;

        const auto delimiterCount = std::ranges::count(formatSpecifier, ':');
        if (delimiterCount == 1) {
            const auto parts = wolv::util::splitString(std::string(formatSpecifier), ":");
            if (parts.size() != 2)
                return std::nullopt;

            const auto result = this->evaluateExpression(parts[0], address, bytes, variables);
            if (!result.has_value())
                return std::nullopt;

            formatString = fmt::format("{{0:{}}}", parts[1]);
            value = result.value();
        } else if (delimiterCount > 1) {
            return std::nullopt;
        } else {
            if (formatSpecifier.contains('[')) {
                auto result = evaluateLUTFormatSpecifier(formatSpecifier, tables, bytes);
                if (!result.has_value())
                    return std::nullopt;
                formatString = result.value();
            } else {
                formatString = "{}";
                value = m_bitPattern.valueOf(formatSpecifier[0], bytes);
            }
        }

        return fmt::vformat(formatString, fmt::make_format_args(value));
    }

    std::optional<std::string> Opcode::format(u64 address, const std::span<const u8> bytes, const std::map<std::string, std::vector<std::string>> &tables, const std::map<std::string, i64> &variables) const {
        if (!m_ifSpec.empty()) {
            if (evaluateExpression(m_ifSpec, address, bytes, variables).value_or(0) == 0)
                return std::nullopt;
        }

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

                if (auto formattedValue = evaluateFormatSpecifier(formatSpecifier, tables, variables, address, bytes); formattedValue.has_value()) {
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

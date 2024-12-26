#include <disasm/spec/bit_pattern.hpp>
#include <disasm/types.hpp>

#include <algorithm>
#include <ranges>
#include <cctype>

namespace disasm::spec {

    namespace {

        bool getBit(const std::span<const u8> bytes, u32 bitOffset) {
            return bytes[bitOffset / 8] & (0x80 >> (bitOffset % 8));
        }

        bool isMaskCharacter(char c) {
            return c == '0' || c == '1' || std::isalpha(c);
        }

    }

    BitPattern::BitPattern(std::string mask) : m_mask(std::move(mask)) {
        for (char c : m_mask) {
            if (isMaskCharacter(c) && std::find(m_placeholders.begin(), m_placeholders.end(), c) == m_placeholders.end())
                m_placeholders.push_back(c);
        }
    }

    bool BitPattern::matches(const std::span<const u8> bytes) const {
        u32 bitOffset = 0;
        for (size_t maskOffset = 0; maskOffset < m_mask.size(); maskOffset += 1) {
            const char c = m_mask[maskOffset];

            if (c == '\'' || c == ' ') {
                continue;
            } else if (c == '0') {
                if (getBit(bytes, bitOffset))
                    return false;
            } else if (c == '1') {
                if (!getBit(bytes, bitOffset))
                    return false;
            } else {
                // Placeholder character, can be any value
            }

            bitOffset += 1;
        }

        return true;
    }

    u64 BitPattern::valueOf(char placeholder, const std::span<const u8> bytes) const {
        u64 result = 0;

        u32 bitOffset = 0;
        for (size_t maskOffset = 0; maskOffset < m_mask.size(); maskOffset += 1) {
            const char c = m_mask[maskOffset];
            if (c == placeholder) {
                result <<= 1;
                result |= getBit(bytes, bitOffset) ? 1 : 0;
            }

            if (isMaskCharacter(c))
                bitOffset += 1;
        }

        return result;
    }

    u32 BitPattern::getSize() const {
        return (std::ranges::count_if(m_mask.begin(), m_mask.end(), isMaskCharacter) + 7) / 8;
    }

}


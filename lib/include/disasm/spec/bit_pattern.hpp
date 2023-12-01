#pragma once

#include <disasm/types.hpp>

#include <span>
#include <string>
#include <vector>

namespace disasm::spec {

    class BitPattern {
    public:
        explicit BitPattern(std::string mask);

        [[nodiscard]] bool matches(std::span<const u8> bytes) const;
        [[nodiscard]] u64 valueOf(char placeholder, std::span<const u8> bytes) const;

        [[nodiscard]] u32 getSize() const;
        [[nodiscard]] const std::vector<char>& getPlaceholders() const noexcept { return m_placeholders; }

    private:
        std::string m_mask;
        std::vector<char> m_placeholders;
    };

}
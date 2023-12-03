#include <disasm/spec/spec.hpp>

#include <format>

namespace disasm::spec {

    namespace {

        struct FormatResult {
            std::string mnemonic;
            std::string operands;
            const Opcode *opcode;
            size_t advance;
        };

        FormatResult formatOpcode(const std::vector<Opcode> &opcodes, u64 address, std::span<const u8> bytes, bool allowMultiple = false) {
            std::string mnemonic, operands;
            size_t advance = 0;

            const Opcode *foundOpcode = nullptr;
            for (const Opcode &opcode : opcodes) {
                if (auto formattedValue = opcode.format(address, bytes.subspan(advance)); formattedValue.has_value()) {
                    mnemonic += opcode.getMnemonic();
                    mnemonic.append(" ");

                    operands += formattedValue.value();
                    operands.append(" ");

                    advance += opcode.getSize();

                    foundOpcode = &opcode;

                    if (!allowMultiple)
                        break;
                }
            }

            return { mnemonic, operands, foundOpcode, advance };
        }

    }

    std::vector<Disassembly> Spec::disassemble(const std::span<const u8> bytes) const {
        std::vector<Disassembly> disassembly;

        size_t offset = 0x00;
        std::string mnemonic, operands;
        while (offset < bytes.size()) {
            {
                const auto result = formatOpcode(getPrefixes(), offset, bytes.subspan(offset), true);
                mnemonic += result.mnemonic;
                operands += result.operands;
                offset += result.advance;
            }

            {
                const auto result = formatOpcode(getOpcodes(), offset, bytes.subspan(offset));
                mnemonic += result.mnemonic;
                operands += result.operands;
                offset += result.advance;

                if (result.advance == 0) {
                    operands.clear();
                    disassembly.emplace_back("???");
                    offset += 1;
                } else {
                    std::vector<nlohmann::json> metadata;

                    if (result.opcode != nullptr) {
                        if (auto json = result.opcode->getMetadata(); !json.empty()) {
                            for (auto &entry : json) {
                                metadata.push_back(entry);
                            }
                        }
                    }

                    disassembly.emplace_back(std::move(mnemonic), std::move(operands), std::move(metadata));
                }
            }

        }

        return disassembly;
    }

}


#include <disasm/disasm.hpp>

#include <format>

int main() {
    using namespace disasm;

    const auto loadPath = wolv::io::fs::getExecutablePath()->parent_path() / ".." / ".." / "specs";

    spec::Spec spec = spec::Loader::load(
        std::fs::path("8051/base.json"),
        {loadPath }
    );

    std::vector<disasm::u8> bytes = {
        0x00, 0x01, 0xAA, 0x06, 0x05, 0xBB, 0x0A, 0x0C, 0x0D, 0x0E, 0x0F
    };

    const auto result = spec.disassemble(bytes);

    std::setbuf(stdout, nullptr);
    for (const auto &disassembly : result) {
        std::printf("%s %s\n", disassembly.mnemonic.c_str(), disassembly.operands.c_str());
    }
}

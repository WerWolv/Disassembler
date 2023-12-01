#include <disasm/spec/loader.hpp>
#include <disasm/types.hpp>

#include <wolv/io/file.hpp>

#include <nlohmann/json.hpp>

namespace disasm::spec {

    Spec Loader::load(const std::fs::path& path, const std::vector<std::fs::path> &includeDirectories) {
        for (const auto &includeDirectory : includeDirectories) {
            auto loadPath = includeDirectory / path;
            if (wolv::io::fs::exists(loadPath)) {
                wolv::io::File file(loadPath, wolv::io::File::Mode::Read);

                return load(file.readString(), includeDirectories);
            }
        }

        throw std::runtime_error("Could not find file: " + path.string());
    }

    Spec Loader::load(const std::string& string, const std::vector<std::fs::path> &includeDirectories) {
        auto json = nlohmann::json::parse(string);

        std::vector<Opcode> opcodes;
        for (const auto &includePath : json["includes"].get<std::vector<std::string>>()) {
            Spec includedSpec = load(includePath, includeDirectories);
            std::ranges::copy(includedSpec.getOpcodes(), std::back_inserter(opcodes));
        }

        for (const auto &opcode : json["opcodes"]) {
            opcodes.emplace_back(opcode["mask"], opcode["format"]);
        }

        return { json["name"], opcodes };
    }



}


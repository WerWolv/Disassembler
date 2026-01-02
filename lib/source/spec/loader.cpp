#include <disasm/spec/loader.hpp>
#include <disasm/types.hpp>

#include <wolv/io/file.hpp>

#include <nlohmann/json.hpp>
#include <fmt/format.h>

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
        try {
            auto json = nlohmann::json::parse(string);

            std::vector<Opcode> prefixes;
            std::vector<Opcode> opcodes;
            for (const auto &includePath : json["includes"].get<std::vector<std::string>>()) {
                Spec includedSpec = load(includePath, includeDirectories);
                std::ranges::copy(includedSpec.getOpcodes(), std::back_inserter(opcodes));
            }

            std::map<std::string, std::vector<std::string>> tables;
            if (json.contains("tables")) {
                for (const auto &object : json["tables"].items()) {
                    const auto &name = object.key();
                    const auto &table = object.value();

                    std::vector<std::string> outputTable;
                    for (const auto &entry : table) {
                        outputTable.emplace_back(entry.get<std::string>());
                    }
                    tables.emplace(name, std::move(outputTable));
                }
            }

            for (const auto &opcode : json["opcodes"]) {
                opcodes.emplace_back(opcode["mnemonic"], opcode["mask"], opcode["format"], opcode.contains("metadata") ? opcode["metadata"] : nlohmann::json());
            }

            for (const auto &opcode : json["prefixes"]) {
                prefixes.emplace_back(opcode["mnemonic"], opcode["mask"], opcode["format"], opcode.contains("metadata") ? opcode["metadata"] : nlohmann::json());
            }

            return Spec {
                json["name"],
                std::move(opcodes),
                std::move(prefixes),
                std::move(tables)
            };
        } catch (const nlohmann::json::exception &e) {
            throw std::runtime_error(fmt::format("Error while parsing file: {}", e.what()));
        }
    }

}


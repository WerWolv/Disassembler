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

            std::vector<std::unique_ptr<Setting>> settings;
            if (json.contains("settings") && json["settings"].is_array()) {
                for (const auto &entry : json["settings"].items()) {
                    const auto &setting = entry.value();

                    const auto type = setting["type"].get<std::string>();

                    if (type == "bool") {
                        auto newSetting = std::make_unique<SettingBool>();
                        settings.emplace_back(std::move(newSetting));
                    } else if (type == "int") {
                        auto newSetting = std::make_unique<SettingInt>();
                        newSetting->min = setting["min"].get<i64>();
                        newSetting->max = setting["max"].get<i64>();

                        const auto radix = setting.contains("radix") ? setting["radix"].get<std::string>() : "dec";
                        if (radix == "hex") {
                            newSetting->radix = SettingInt::Radix::Hexadecimal;
                        } else if (radix == "dec") {
                            newSetting->radix = SettingInt::Radix::Decimal;
                        } else {
                            throw std::runtime_error(fmt::format("Unknown radix type: {}", radix));
                        }

                        settings.emplace_back(std::move(newSetting));
                    } else if (type == "enum") {
                        auto newSetting = std::make_unique<SettingEnum>();

                        for (const auto &value : setting["values"]) {
                            SettingEnum::Value enumValue;
                            enumValue.name = value["name"].get<std::string>();
                            enumValue.value = value["value"].get<i64>();
                            newSetting->values.emplace_back(std::move(enumValue));
                        }

                        settings.emplace_back(std::move(newSetting));
                    } else {
                        throw std::runtime_error(fmt::format("Unknown setting type: {}", type));
                    }

                    auto &newSetting = settings.back();
                    newSetting->name = setting["name"].get<std::string>();
                    newSetting->displayName = setting["displayName"].get<std::string>();
                }
            }

            for (const auto &opcode : json["opcodes"]) {
                opcodes.emplace_back(opcode["mnemonic"], opcode["mask"], opcode["format"], opcode.contains("sizeof") ? opcode["sizeof"] : "", opcode.contains("metadata") ? opcode["metadata"] : nlohmann::json());
            }

            for (const auto &opcode : json["prefixes"]) {
                prefixes.emplace_back(opcode["mnemonic"], opcode["mask"], opcode["format"], opcode.contains("sizeof") ? opcode["sizeof"] : "", opcode.contains("metadata") ? opcode["metadata"] : nlohmann::json());
            }

            return Spec {
                json["name"].get<std::string>(),
                std::move(opcodes),
                std::move(prefixes),
                std::move(tables),
                std::move(settings)
            };
        } catch (const std::exception &e) {
            throw std::runtime_error(fmt::format("Error while parsing file: {}", e.what()));
        }
    }

}


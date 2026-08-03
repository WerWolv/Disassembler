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

    namespace {

        Opcode parseOpcode(const nlohmann::json& json) {
            return {
                json["mnemonic"],
                json["mask"],
                json["format"],
                json.contains("sizeof") ? json["sizeof"] : "",
                json.contains("if") ? json["if"] : "",
                json.contains("metadata") ? json["metadata"] : nlohmann::json()
            };
        }

        std::map<std::string, std::vector<std::string>> parseTables(const nlohmann::json& json) {
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

            return tables;
        }

        std::vector<std::unique_ptr<Setting>> parseSettings(const nlohmann::json& json) {
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

            return settings;
        }

    }

    Spec Spec::mergeSpecs(std::vector<Spec> &&specs) {
        Spec resultSpec;

        for (auto &spec : specs) {
            std::ranges::move(spec.m_opcodes, std::back_inserter(resultSpec.m_opcodes));
            std::ranges::move(spec.m_prefixes, std::back_inserter(resultSpec.m_prefixes));
            std::ranges::move(spec.m_tables, std::inserter(resultSpec.m_tables, resultSpec.m_tables.end()));
            std::ranges::move(spec.m_settings, std::back_inserter(resultSpec.m_settings));
            resultSpec.m_name = spec.m_name;
        }

        return resultSpec;
    }

    Spec Loader::load(const std::string& string, const std::vector<std::fs::path> &includeDirectories) {
        try {
            auto json = nlohmann::json::parse(string);

            std::vector<Spec> specs;
            for (const auto &includePath : json["includes"].get<std::vector<std::string>>()) {
                specs.emplace_back(load(includePath, includeDirectories));
            }

            auto tables = parseTables(json);
            auto settings = parseSettings(json);

            std::vector<Opcode> opcodes;
            for (const auto &opcode : json["opcodes"]) {
                opcodes.emplace_back(parseOpcode(opcode));
            }

            std::vector<Opcode> prefixes;
            for (const auto &opcode : json["prefixes"]) {
                prefixes.emplace_back(parseOpcode(opcode));
            }

            specs.emplace_back(
                json["name"].get<std::string>(),
                std::move(opcodes),
                std::move(prefixes),
                std::move(tables),
                std::move(settings)
            );

            return Spec::mergeSpecs(std::move(specs));
        } catch (const std::exception &e) {
            throw std::runtime_error(fmt::format("Error while parsing file: {}", e.what()));
        }
    }

}


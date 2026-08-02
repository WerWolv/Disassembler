#pragma once

#include <disasm/spec/opcode.hpp>

#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace disasm::spec {

    struct Disassembly {
        std::string mnemonic;
        std::string operands;
        std::span<const u8> bytes;
        std::vector<nlohmann::json> metadata;
    };

    struct Setting {
        virtual ~Setting() = default;

        std::string name;
        std::string displayName;

        class Visitor;
        virtual void accept(Visitor& visitor) = 0;
    };

    struct SettingBool;
    struct SettingInt;
    struct SettingEnum;

    class Setting::Visitor {
    public:
        virtual ~Visitor() = default;

        virtual void visit(SettingBool& setting) = 0;
        virtual void visit(SettingInt& setting)  = 0;
        virtual void visit(SettingEnum& setting) = 0;
    };

    struct SettingBool : Setting {
        void accept(Visitor& visitor) override {
            visitor.visit(*this);
        }
    };

    struct SettingEnum : Setting {
        struct Value {
            std::string name;
            i64 value;
        };

        std::vector<Value> values;

        void accept(Visitor& visitor) override {
            visitor.visit(*this);
        }
    };

    struct SettingInt : Setting {
        enum class Radix { Decimal, Hexadecimal } radix;
        i64 min, max;

        void accept(Visitor& visitor) override {
            visitor.visit(*this);
        }
    };

    class Spec {
    public:
        Spec() = default;
        Spec(std::string name, std::vector<Opcode> opcodes, std::vector<Opcode> prefixes, std::map<std::string, std::vector<std::string>> tables, std::vector<std::unique_ptr<Setting>> &&settings)
            : m_name(std::move(name)),
              m_opcodes(std::move(opcodes)),
              m_prefixes(std::move(prefixes)),
              m_tables(std::move(tables)),
              m_settings(std::move(settings)) { }

        Spec(const Spec &) = delete;
        Spec &operator=(const Spec &) = delete;

        Spec(Spec &&) = default;
        Spec &operator=(Spec &&) = default;

        [[nodiscard]] const std::string& getName() const noexcept { return m_name; }
        [[nodiscard]] const std::vector<Opcode>& getOpcodes() const noexcept { return m_opcodes; }
        [[nodiscard]] const std::vector<Opcode>& getPrefixes() const noexcept { return m_prefixes; }
        [[nodiscard]] const std::map<std::string, std::vector<std::string>>& getTables() const noexcept { return m_tables; }
        [[nodiscard]] const std::vector<std::unique_ptr<Setting>>& getSettings() const noexcept { return m_settings; }

        [[nodiscard]] std::vector<Disassembly> disassemble(std::span<const u8> bytes, std::size_t instructionCount = 0, const std::map<std::string, i64> &variables = {}) const;

        static Spec mergeSpecs(std::vector<Spec> &&specs);
    private:
        std::string m_name;
        std::vector<Opcode> m_opcodes, m_prefixes;
        std::map<std::string, std::vector<std::string>> m_tables;
        std::vector<std::unique_ptr<Setting>> m_settings;
    };

}

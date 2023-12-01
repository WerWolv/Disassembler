#pragma once

#include <disasm/spec/spec.hpp>

#include <wolv/io/fs.hpp>

namespace disasm::spec {

    class Loader {
    public:
        static Spec load(const std::fs::path &path, const std::vector<std::fs::path> &includeDirectories);
        static Spec load(const std::string &string, const std::vector<std::fs::path> &includeDirectories);

    private:
        Loader() = default;
    };

}
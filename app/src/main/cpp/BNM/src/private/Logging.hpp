#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>
#include <memory>

namespace BNM::Internal {
    void SetupLogging();
    extern std::shared_ptr<spdlog::logger> bnmLogger;
}

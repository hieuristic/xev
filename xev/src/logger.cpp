#include <xev/logger.h>

namespace xev {

void InitLogger() {
    spdlog::set_pattern("%^%Y-%m-%d %H:%M:%S [%L] %v%$");
}

} // namespace xev

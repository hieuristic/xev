#ifndef XEV_LOGGER_H
#define XEV_LOGGER_H

#include <spdlog/spdlog.h>

namespace xev {

void InitLogger();

#define XEV_INFO(...) spdlog::info(__VA_ARGS__)
#define XEV_ERROR(...) spdlog::error(__VA_ARGS__)
#define XEV_WARN(...) spdlog::warn(__VA_ARGS__)
#define XEV_DEBUG(...) spdlog::debug(__VA_ARGS__)

#define XEV_ERROR_IF_FAILED(res_, ...) \
    do {                               \
        if ((res_) != VK_SUCCESS) {    \
            XEV_ERROR(__VA_ARGS__);    \
        }                              \
    } while (0)

} // namespace xev

#endif // XEV_LOGGER_H

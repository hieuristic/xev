#ifndef XEV_LOGGER_H
#define XEV_LOGGER_H

#include <spdlog/spdlog.h>

namespace xev {

void InitLogger();

#define XEV_INFO(...) spdlog::info(__VA_ARGS__)
#define XEV_STR_(x) #x
#define XEV_STR(x) XEV_STR_(x)
#define XEV_ERROR(fmt, ...) spdlog::error("[" __FILE__ ":" XEV_STR(__LINE__) "] " fmt, ##__VA_ARGS__)
#define XEV_WARN(...) spdlog::warn(__VA_ARGS__)
#define XEV_DEBUG(...) spdlog::debug(__VA_ARGS__)

#define XEV_ASSERT(expr)                                                  \
    do {                                                                  \
        if (!(expr)) {                                                    \
            spdlog::critical("[" __FILE__ ":" XEV_STR(__LINE__) "] "      \
                             "Assertion failed: " #expr);                 \
            std::abort();                                                 \
        }                                                                 \
    } while (0)

#define XEV_ERROR_IF_FAILED(res_, ...) \
    do {                               \
        if ((res_) != VK_SUCCESS) {    \
            XEV_ERROR(__VA_ARGS__);    \
        }                              \
    } while (0)

} // namespace xev

#endif // XEV_LOGGER_H

#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <cstdlib>
#include <iostream>
#include <execinfo.h>

#define VMA_ASSERT(expr)                                                   \
  do {                                                                     \
    if (!(expr)) {                                                         \
      std::cerr << "VMA Assertion failed: " << #expr << "\n";              \
      void* callstack[128];                                                \
      int frames = backtrace(callstack, 128);                              \
      char** strs = backtrace_symbols(callstack, frames);                  \
      std::cerr << "Stack trace:\n";                                       \
      for (int i = 0; i < frames; ++i) {                                   \
        std::cerr << strs[i] << "\n";                                      \
      }                                                                    \
      free(strs);                                                          \
      std::abort();                                                        \
    }                                                                      \
  } while (0)

#include <vk_mem_alloc.h>

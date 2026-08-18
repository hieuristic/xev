#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace xev {

struct Mount {
  virtual std::vector<uint8_t> read(std::string_view path,
                                    uint64_t offset,
                                    uint64_t count) = 0;
  virtual void index() = 0;
  std::unordered_map<std::string, uint32_t> directive;
  std::string mntPath;
  bool isWritable;
};

}  // namespace xev

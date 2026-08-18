#pragma once
#include <vector>

namespace xev {

struct IMount {
  virtual std::vector<uint8_t> read();
  virtual void index() = 0;
  std::string srcPath;
  bool isWritable;
};

}

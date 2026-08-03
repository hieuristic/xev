#pragma once
#include <vector>

namespace xev {

struct FileSource {
  virtual std::vector<uint8_t> read();
  virtual void index() = 0;
  std::string srcPath;
};

}

#pragma once
#include <string_view>

/*
 * Unlike arxiv.h which is the interface for file reading. This
 * Define the layout for the actual .arx file. Method for writing,
 * and reading .arx are included.
*/

namespace xev {

// XEV's Archival File
struct ARX {
  ARK(std::string_view);
  ~ARK();
}

}

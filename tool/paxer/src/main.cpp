#include <unistd.h>
#include <xev/filesystem/arxiv.h>
#include <xev/logger.h>
#include <filesystem>
#include <iostream>

void help() {
  std::cout << "Usage: ./paxer -i <input_dir> -o <output_path>\n"
            << "  -i <dir>   Target directory containing assets to pack\n"
            << "  -o <file>  Output .arx archive file or directory\n";
}

bool parse(int argc,
           char* argv[],
           std::filesystem::path& tgtDir,
           std::filesystem::path& outFile) {
  int opt;
  while ((opt = getopt(argc, argv, "i:o:h")) != -1) {
    switch (opt) {
      case 'i':
        tgtDir = optarg;
        break;
      case 'o':
        outFile = optarg;
        break;
      default:
        help();
        return false;
    }
  }

  if (tgtDir.empty() || outFile.empty()) {
    help();
    return false;
  }
  if (outFile.extension() != ".arx") {
    outFile /= "f0.arx";
  }

  return true;
}

int main(int argc, char* argv[]) {
  std::filesystem::path tgtDir, outFile;
  if (!parse(argc, argv, tgtDir, outFile))
    return 1;

  bool res = (xev::ArxivMount::pack(tgtDir, outFile));
  if (res) {
    XEV_INFO("Succesfully packed! {} -> {}", tgtDir.string(), outFile.string());
  } else {
    XEV_ERROR("Failed to pack");
    return 1;
  }

  return 0;
}


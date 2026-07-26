#include <xev/ui/font.h>
#include <fstream>

namespace xev {

Font::Font(const ResourceManager& manager,
           const char* atlas_path,
           const char* config_path)
    : m_manager(manager) {
  fontAtlas.load(path);
  m_manager.alloc(fontAtlas);
}

Font::~Font() {
  m_manager.free(fontAtlas);
}

void Font::bind(const GlobalDescriptorSet& desc_set) {
  m_texid = desc_set.set(fontAtlas);
}

}  // namespace xev

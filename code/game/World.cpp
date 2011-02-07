#include "World.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
World::World(unsigned width, unsigned height)
///////////////////////////////////////////////////////////////////////////////
  : m_width(width),
    m_height(height)
{
  for (unsigned i = 0; i < height; ++i) {
    std::vector<WorldTile> row(width);
    m_tiles.push_back(row);
  }
}

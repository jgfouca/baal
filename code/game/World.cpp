#include "World.hpp"

#include <iostream>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
World::World(unsigned width, unsigned height)
///////////////////////////////////////////////////////////////////////////////
  : m_width(width),
    m_height(height)
{
  for (unsigned i = 0; i < height; ++i) {
    std::vector<WorldTile*> row(width, NULL);
    m_tiles.push_back(row);
  }
}

///////////////////////////////////////////////////////////////////////////////
World::~World()
///////////////////////////////////////////////////////////////////////////////
{
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      delete m_tiles[row][col];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
void World::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      m_tiles[row][col]->draw_text(out);
    }
    out << "\n";
  }
}

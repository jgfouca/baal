#ifndef World_hpp
#define World_hpp

#include "WorldTile.hpp"
#include "BaalCommon.hpp"

#include <vector>

namespace baal {

class World
{
 public:
  World(unsigned width, unsigned height);

  // Getters

  /**
   * Return a particular tile
   */
  const WorldTile& get_tile(const Location& location) const {
    return m_tiles[location.row][location.col];
  }

  /**
   * Return a non-const tile
   */
  WorldTile& get_tile(const Location& location) {
    return m_tiles[location.row][location.col];
  }

  unsigned width() const { return m_width; }

  unsigned height() const { return m_height; }

  bool in_bounds(const Location& location) const {
    return location.row < m_height || location.col < m_width;
  }

 private:

  // Members
  unsigned m_width;
  unsigned m_height;
  std::vector<std::vector<WorldTile> > m_tiles;

  // Friend factories
  friend class WorldFactoryGenerated;
  friend class WorldFactoryHardcoded;
};

}

#endif

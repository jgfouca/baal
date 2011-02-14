#ifndef World_hpp
#define World_hpp

#include "WorldTile.hpp"
#include "BaalCommon.hpp"

#include <vector>

namespace baal {

class Disaster;

class World
{
 public:
  World(unsigned width, unsigned height);

  /**
   * Apply a disaster to the world, returns number of casualties.
   */
  unsigned apply_disaster(const Disaster& disaster);

  // Getters

  /**
   * Return a particular tile
   */
  const WorldTile& get_tile(const Location& location) const {
    return m_tiles[location.row][location.col];
  }

  unsigned width() const { return m_width; }

  unsigned height() const { return m_height; }

 private:
  /**
   * Return a non-const reference to a tile, this is private because it should
   * only be used by the world factories which are friends.
   */
  WorldTile& get_tile(const Location& location) {
    return m_tiles[location.row][location.col];
  }

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

#ifndef World_hpp
#define World_hpp

#include "WorldTile.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "Drawable.hpp"
#include "Time.hpp"

#include <vector>
#include <iosfwd>

namespace baal {

class Anomaly;

/**
 * Represents the world.
 */
class World : public Drawable
{
 public:
  World(unsigned width, unsigned height);

  ~World();

  void cycle_turn();

  bool in_bounds(const Location& location) const {
    return location.row < m_height || location.col < m_width;
  }

  // Getters

  /**
   * Return a particular tile
   */
  const WorldTile& get_tile(const Location& location) const {
    Assert(in_bounds(location), "Out of bounds");
    Assert(m_tiles[location.row][location.col] != NULL, "Null");
    return *(m_tiles[location.row][location.col]);
  }

  /**
   * Return a non-const tile
   */
  WorldTile& get_tile(const Location& location) {
    Assert(in_bounds(location), "Out of bounds");
    Assert(m_tiles[location.row][location.col] != NULL, "Null");
    return *(m_tiles[location.row][location.col]);
  }

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /* TODO */ }

  unsigned width() const { return m_width; }

  unsigned height() const { return m_height; }

 private:
  void clear_anomalies();

  // Members
  unsigned m_width;
  unsigned m_height;
  std::vector<std::vector<WorldTile*> > m_tiles;
  Time m_time;
  std::vector<const Anomaly*> m_recent_anomalies;

  // Friend factories
  friend class WorldFactoryGenerated;
  friend class WorldFactoryHardcoded;
  friend class WorldFactoryFromFile;
};

}

#endif

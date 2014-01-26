#ifndef World_hpp
#define World_hpp

#include "WorldTile.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "Time.hpp"
#include "City.hpp"

#include <vector>
#include <iosfwd>
#include <libxml/parser.h>
#include <boost/iterator/filter_iterator.hpp>

namespace baal {

class Anomaly;
class Engine;

/**
 * Represents the world.
 */
class World
{
  struct ValidFilter
  {
    ValidFilter(World const& world) : m_world(world) {}

    bool operator()(Location const& loc) const
    { return m_world.in_bounds(loc); }

    World const& m_world;
  };

 public:
  typedef boost::filter_iterator<ValidFilter, LocationIterator> ValidNearbyTileIterator;
  typedef boost::iterator_range<ValidNearbyTileIterator> ValidNearbyTileRange;

  World(unsigned width, unsigned height, Engine& engine);

  ~World();

  World(const World&) = delete;
  World& operator=(const World&) = delete;

  bool in_bounds(const Location& location) const {
    return location.row < m_height && location.col < m_width;
  }

  // Getters

  /**
   * Return a particular tile
   */
  const WorldTile& get_tile(const Location& location) const {
    Assert(in_bounds(location), "Out of bounds");
    Assert(m_tiles[location.row][location.col] != nullptr, "Null");
    return *(m_tiles[location.row][location.col]);
  }

  /**
   * Return a non-const tile
   */
  WorldTile& get_tile(const Location& location) {
    Assert(in_bounds(location), "Out of bounds");
    Assert(m_tiles[location.row][location.col] != nullptr, "Null");
    return *(m_tiles[location.row][location.col]);
  }

  unsigned width() const { return m_width; }

  unsigned height() const { return m_height; }

  const std::vector<City*>& cities() const { return m_cities; }

  const Time& time() const { return m_time; }

  const std::vector<std::shared_ptr<const Anomaly>> anomalies() const
  { return m_recent_anomalies; }

  // Modification API

  void cycle_turn();

  void place_city(const Location& location, const std::string& name = "");

  void remove_city(City& city);

  xmlNodePtr to_xml();

  ValidNearbyTileRange valid_nearby_tile_range(const Location& center, unsigned radius = 1) const;

 private:

  // Members
  unsigned m_width;
  unsigned m_height;
  std::vector<std::vector<WorldTile*> > m_tiles;
  Time m_time;
  std::vector<std::shared_ptr<const Anomaly>> m_recent_anomalies;
  std::vector<City*> m_cities;
  Engine& m_engine;

  // Friend factories
  friend class WorldFactoryGenerated;
  friend class WorldFactoryHardcoded;
  friend class WorldFactoryFromFile;
};

}

#endif

#ifndef City_hpp
#define City_hpp

#include "BaalCommon.hpp"

#include <string>
#include <list>
#include <libxml/parser.h>

namespace baal {

class WorldTile;
class LandTile;

/**
 * Represents human-built cities.
 * TODO: describe conceptually how cities work.
 */
class City
{
 public:

  //
  // ==== Public API ====
  //

  City(const std::string& name, const Location& location);

  City & operator=(const City&) = delete;
  City(const City&)             = delete;
  City()                        = delete;

  //
  // Query / Getter API
  //

  const std::string& name() const { return m_name; }

  unsigned population() const { return m_population; }

  unsigned rank() const { return m_rank; }

  const Location& location() const { return m_location; }

  bool famine() const { return m_famine; }

  unsigned defense() const { return m_defense_level; }

  xmlNodePtr to_xml() const;

  //
  // Modification API
  //

  /**
   * Tell this city that the turn is cycling
   */
  void cycle_turn();

  /**
   * Kill off some of this city's citizens
   */
  void kill(unsigned killed);

  /**
   * Destroy city defenses, lowering them by an amount equal to the
   * 'levels' parameter.
   */
  void destroy_defense(unsigned levels);

 private:

  //
  // ==== Internal methods ====
  //

  /**
   * Add a level of infrastructure to a tile.
   */
  bool build_infra(LandTile& land_tile);

  //
  // ==== Members ====
  //

  std::string m_name;
  unsigned    m_rank;
  unsigned    m_population;
  unsigned    m_next_rank_pop;
  float       m_production;
  Location    m_location;
  unsigned    m_defense_level;
  bool        m_famine;

 public:
  //
  // ==== Class constants ====
  //

  // City growth/food/production contants
  static constexpr float CITY_BASE_GROWTH_RATE = 0.01; // 1% per turn
  static constexpr float MAX_GROWTH_MODIFIER   = 4.0;
  static constexpr unsigned CITY_RANK_UP_MULTIPLIER = 2;
  static constexpr unsigned CITY_STARTING_POP = 1000;
  static constexpr unsigned MIN_CITY_SIZE = CITY_STARTING_POP / 5;
  static constexpr unsigned POP_THAT_EATS_ONE_FOOD = 1000;
  static constexpr float FOOD_FROM_CITY_CENTER = 1.0;
  static constexpr float PROD_FROM_CITY_CENTER = 1.0;
  static constexpr float PROD_FROM_SPECIALIST  = 1.0;

  // Constants for production costs of various buildable items
  static constexpr unsigned SETTLER_PROD_COST  = 200;
  static constexpr unsigned INFRA_PROD_COST    = 50;
  static constexpr unsigned CITY_DEF_PROD_COST = 400;

  // AI constants
  static constexpr float TOO_MANY_FOOD_WORKERS = 0.66;
  static constexpr float PROD_BEFORE_SETTLER   = 7.0;
};

}

#endif

#ifndef City_hpp
#define City_hpp

#include "BaalCommon.hpp"

#include <string>
#include <list>

namespace baal {

class WorldTile;
class LandTile;

/**
 * Represents human-built cities.
 */
class City
{
 public:
  const std::string& name() const { return m_name; }

  void cycle_turn();

  unsigned population() const { return m_population; }

  unsigned rank() const { return m_rank; }

  const Location& location() const { return m_location; }

  bool famine() const { return m_famine; }

  unsigned defense() const { return m_defense_level; }

 private:

  // Internal methods

  void ordered_insert(std::list<WorldTile*>& tile_list, WorldTile& tile) const;

  bool build_infra(LandTile& land_tile);

  // Members

  std::string m_name;
  unsigned    m_rank;
  unsigned    m_population;
  unsigned    m_next_rank_pop;
  float       m_production;
  Location    m_location;
  unsigned    m_defense_level;
  bool        m_famine;

  // Behavioral constants

  static const float CITY_BASE_GROWTH_RATE = 0.01; // 1% per turn
  static const float MAX_GROWTH_MODIFIER   = 4.0;
  static const unsigned CITY_RANK_UP_MULTIPLIER = 2;
  static const unsigned CITY_STARTING_POP = 1000;
  static const unsigned POP_THAT_EATS_ONE_FOOD = 1000;
  static const float FOOD_FROM_CITY_CENTER = 1.0;
  static const float PROD_FROM_CITY_CENTER = 1.0;
  static const float PROD_FROM_SPECIALIST  = 1.0;
  static const unsigned SETTLER_PROD_COST  = 200;
  static const unsigned INFRA_PROD_COST    = 50;
  static const unsigned CITY_DEF_PROD_COST = 400;
  static const float TOO_MANY_FOOD_WORKERS = 0.66;
  static const float PROD_BEFORE_SETTLER   = 7.0;

  // Friend interface

  City(const std::string& name, const Location& location);

  void kill(unsigned killed);

  // Friends

  friend class Spell;
  friend class World;
};

}

#endif

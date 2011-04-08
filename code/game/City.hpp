#ifndef City_hpp
#define City_hpp

#include "BaalCommon.hpp"

#include <string>
#include <list>

namespace baal {

class World;
class WorldTile;
class LandTile;

/**
 * Represents human-built cities.
 */
class City
{
 public:
  City(const std::string& name, World& world, const Location& location);

  const std::string& name() const { return m_name; }

  void cycle_turn();

  unsigned population() const { return m_population; }

  unsigned rank() const { return m_rank; }

 private:

  // Internal methods

  void ordered_insert(std::list<WorldTile*>& tile_list, WorldTile& tile) const;

  bool try_to_build_infra(LandTile& land_tile);

  // Members

  std::string m_name;
  unsigned    m_rank;
  unsigned    m_population;
  unsigned    m_next_rank_pop;
  float       m_production;
  World&      m_world;
  Location    m_city_location;

  // Behavioral constants

  static const float CITY_BASE_GROWTH_RATE = 0.01; // 1% per turn
  static const float MAX_GROWTH_MODIFIER   = 4.0;
  static const unsigned CITY_RANK_UP_MULTIPLIER = 2;
  static const unsigned CITY_STARTING_POP = 1000;
  static const unsigned POP_THAT_EATS_ONE_FOOD = 1000;
  static const float FOOD_FROM_CITY_CENTER = 1.0;
  static const float PROD_FROM_CITY_CENTER = 1.0;
  static const float PROD_FROM_SPECIALIST  = 1.0;
  static const unsigned SETTLER_PROD_COST  = 100;
  static const unsigned INFRA_PROD_COST    = 25;
  static const unsigned CITY_DEF_PROD_COST = 50;
  static const float TOO_MANY_FOOD_WORKDERS = 0.66;
  static const float EXPECTED_PROD_PER_SIZE = 2.0;

  // Friend interface

  void kill(unsigned killed);

  // Friends

  friend class Spell;
};

}

#endif

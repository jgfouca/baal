#ifndef City_hpp
#define City_hpp

#include <string>

namespace baal {

/**
 * Represents human-built cities.
 */
class City
{
 public:
  City(const std::string& name);

  const std::string& name() const { return m_name; }

  void cycle_turn();

  unsigned population() const { return m_population; }

  unsigned rank() const { return m_rank; }

 private:

  // Members

  std::string m_name;
  unsigned    m_rank;
  unsigned    m_population;
  unsigned    m_next_rank_pop;
  float       m_production;

  // Behavioral constants

  static const float CITY_BASE_GROWTH_RATE = 0.01; // 1% per turn
  static const float MAX_GROWTH_MODIFIER   = 4.0;
  static const unsigned CITY_RANK_UP_MULTIPLIER = 2;
  static const unsigned CITY_STARTING_POP = 1000;
  static const unsigned POP_THAT_EATS_ONE_FOOD = 1000;

  // Friend interface

  void kill(unsigned killed);

  // Friends

  friend class Spell;
};

}

#endif

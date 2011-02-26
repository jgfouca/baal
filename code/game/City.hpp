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

  void cycle_turn(float food_gathered);

 private:
  std::string m_name;
  unsigned m_rank;
  unsigned m_population;
  unsigned m_next_rank_pop;

  static const float CITY_BASE_GROWTH_RATE = 1.05; // 5%
  static const unsigned CITY_RANK_UP_MULTIPLIER = 2;
  static const unsigned CITY_STARTING_POP = 1000;
  static const unsigned POP_THAT_EATS_ONE_FOOD = 1000;
};

}

#endif

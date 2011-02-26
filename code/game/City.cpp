#include "City.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
City::City(const std::string& name)
///////////////////////////////////////////////////////////////////////////////
  : m_name(name),
    m_rank(1),
    m_population(CITY_STARTING_POP),
    m_next_rank_pop(CITY_STARTING_POP * CITY_RANK_UP_MULTIPLIER)
{}

///////////////////////////////////////////////////////////////////////////////
void City::cycle_turn(float food_gathered)
///////////////////////////////////////////////////////////////////////////////
{
  float req_food = static_cast<float>(m_population) / POP_THAT_EATS_ONE_FOOD;
  if (food_gathered < req_food) {
    // starvation, no growth
  }
  else {
    float surplus_food = food_gathered - req_food;
    float pop_growth_rate = (1 + surplus_food) * CITY_BASE_GROWTH_RATE;
    m_population += pop_growth_rate * m_population;
    if (m_population > m_next_rank_pop) {
      ++m_rank;
      m_next_rank_pop *= CITY_RANK_UP_MULTIPLIER;
    }
  }
}

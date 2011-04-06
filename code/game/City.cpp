#include "City.hpp"
#include "BaalExceptions.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
City::City(const std::string& name)
///////////////////////////////////////////////////////////////////////////////
  : m_name(name),
    m_rank(1),
    m_population(CITY_STARTING_POP),
    m_next_rank_pop(CITY_STARTING_POP * CITY_RANK_UP_MULTIPLIER),
    m_production(0.0)
{}

///////////////////////////////////////////////////////////////////////////////
void City::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  Require(m_population > 0,
          "This city has no people and should have been deleted");

  // TODO: Gather resources based on nearby worked tiles
  float food_gathered = static_cast<float>(m_population) / POP_THAT_EATS_ONE_FOOD;
  float prod_gathered = 1.0;

  // Accumulate production
  m_production += prod_gathered;

  // Handle population growth

  // Compute multiplier, cannot exceed base by more than a factor of 4
  float req_food = static_cast<float>(m_population) / POP_THAT_EATS_ONE_FOOD;
  float food_multiplier;
  if (food_gathered < req_food) {
    food_multiplier = -req_food / food_gathered;
    if (food_multiplier < -1 * MAX_GROWTH_MODIFIER) {
      food_multiplier = -1 * MAX_GROWTH_MODIFIER;
    }
  }
  else {
    food_multiplier = food_gathered / req_food;
    if (food_multiplier > MAX_GROWTH_MODIFIER) {
      food_multiplier = MAX_GROWTH_MODIFIER;
    }
  }

  float pop_growth_rate = 1 + (food_multiplier * CITY_BASE_GROWTH_RATE);
  m_population *= pop_growth_rate;
  if (m_population > m_next_rank_pop) {
    ++m_rank;
    m_next_rank_pop *= CITY_RANK_UP_MULTIPLIER;
  }
}

///////////////////////////////////////////////////////////////////////////////
void City::kill(unsigned killed)
///////////////////////////////////////////////////////////////////////////////
{
  // Cites can be wiped out, but that is done via the World object that owns
  // them.
  Require(m_population >= killed, "Invalid killed: " << killed);

  m_population -= killed;
}

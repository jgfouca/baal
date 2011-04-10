#include "PlayerAI.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "City.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
PlayerAI::PlayerAI()
///////////////////////////////////////////////////////////////////////////////
  : m_tech_level(STARTING_TECH_LEVEL),
    m_tech_points(0),
    m_next_tech_level_cost(FIRST_TECH_LEVEL_COST),
    m_population(0)
{}

///////////////////////////////////////////////////////////////////////////////
void PlayerAI::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine = Engine::instance();

  // Compute population and manage cities
  m_population = 0;
  const World& world = engine.world();
  const std::vector<City*>& cities = world.cities();
  for (std::vector<City*>::const_iterator
       itr = cities.begin(); itr != cities.end(); ++itr) {
    City* city = *itr;
    m_population += city->population();
  }
  Require(m_population > 0, "This map has no human presence!");

  // Adjust tech based on population
  unsigned tech_points = m_population / POP_PER_TECH_POINT;
  m_tech_points += tech_points;
  if (m_tech_points >= m_next_tech_level_cost) {
    // Level up tech
    ++m_tech_level;
    m_tech_points -= m_next_tech_level_cost; // rollover points
    m_next_tech_level_cost *= TECH_LEVEL_COST_INCREASE;
  }

  // Tech invariants
  Require(m_tech_points < m_next_tech_level_cost,
          "Expect tech-points(" << m_tech_points <<
          ") < tech-cost(" << m_next_tech_level_cost << ")");
}

///////////////////////////////////////////////////////////////////////////////
void PlayerAI::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  out << "AI PLAYER STATS:\n"
      << "  tech level: " << BOLD_COLOR << GREEN << m_tech_level << CLEAR_ALL << "\n"
      << "  population: " << BOLD_COLOR << BLUE << m_population << CLEAR_ALL << "\n";
}

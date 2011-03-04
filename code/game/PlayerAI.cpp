#include "PlayerAI.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "City.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
PlayerAI::PlayerAI(Engine& engine)
///////////////////////////////////////////////////////////////////////////////
  : m_engine(engine),
    m_tech_level(STARTING_TECH_LEVEL),
    m_tech_points(0),
    m_next_tech_level_cost(FIRST_TECH_LEVEL_COST),
    m_population(0)
{}

///////////////////////////////////////////////////////////////////////////////
void PlayerAI::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  // Compute population and manage cities
  m_population = 0;
  const World& world = m_engine.world();
  for (unsigned row = 0; row < world.height(); ++row) {
    for (unsigned col = 0; col < world.width(); ++col) {
      City* city = world.get_tile(Location(row, col)).city();
      if (city) {
        m_population += city->population();

        manage_city(*city);
      }
    }
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
void PlayerAI::manage_city(City& city)
///////////////////////////////////////////////////////////////////////////////
{
  // TODO

  // Allocate workers

  // Decide what to build
}

///////////////////////////////////////////////////////////////////////////////
void PlayerAI::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  out << "AI PLAYER STATS:\n"
      << "  tech level: \033[1;32m" << m_tech_level << "\033[0m \n"
      << "  population: \033[1;34m" << m_population << "\033[0m \n";
}

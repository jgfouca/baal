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
  const World& world = engine.world();
  const std::vector<City*>& cities = world.cities();

  // Manage cities. Note that this may cause additional cities to be created,
  // so we need to store the number of cities at the start of the cycle.
  unsigned num_cities_at_start = cities.size();
  for (unsigned i = 0; i < num_cities_at_start; ++i) {
    City* city = cities[i];;
    city->cycle_turn();
  }

  // Compute population
  m_population = 0;
  for (std::vector<City*>::const_iterator
       itr = cities.begin(); itr != cities.end(); ++itr) {
    City* city = *itr;
    m_population += city->population();
  }

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

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr PlayerAI::to_xml()///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr PlayerAI_node = xmlNewNode(NULL, BAD_CAST "PlayerAI - stub");
  return PlayerAI_node;
}

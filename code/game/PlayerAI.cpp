#include "PlayerAI.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "City.hpp"

namespace baal {

///////////////////////////////////////////////////////////////////////////////
PlayerAI::PlayerAI(const Engine& engine)
///////////////////////////////////////////////////////////////////////////////
  : m_tech_level(STARTING_TECH_LEVEL),
    m_tech_points(0),
    m_population(0),
    m_engine(engine)
{}

///////////////////////////////////////////////////////////////////////////////
void PlayerAI::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  const World& world = m_engine.world();
  const std::vector<City*>& cities = world.cities();

  // Manage cities. Note that this may cause additional cities to be created,
  // so we need to store the number of cities at the start of the cycle.
  for (unsigned i = 0, ie = cities.size(); i < ie; ++i) {
    City* city = cities[i];
    city->cycle_turn();
  }

  // Compute population
  m_population = 0;
  for (const City* city : cities) {
    m_population += city->population();
  }

  // Adjust tech based on population
  const unsigned tech_points = TECH_POINT_FUNC(m_population);
  m_tech_points += tech_points;
  if (m_tech_points >= next_tech_level_cost()) {
    // Level up tech
    m_tech_points -= next_tech_level_cost();
    ++m_tech_level;
  }

  // Tech invariants
  Require(m_tech_points < next_tech_level_cost(),
          "Expect tech-points(" << m_tech_points <<
          ") < tech-cost(" << next_tech_level_cost() << ")");
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr PlayerAI::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr PlayerAI_node = xmlNewNode(nullptr, BAD_CAST "PlayerAI - stub");
  return PlayerAI_node;
}

}

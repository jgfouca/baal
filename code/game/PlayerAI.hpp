#ifndef PlayerAI_hpp
#define PlayerAI_hpp

#include "BaalMath.hpp"

#include <cmath>
#include <libxml/parser.h>

namespace baal {

class City;
class Engine;

/**
 * Manages the "global" (higher-than-city) affairs of the AI. Since most
 * of the AI's "thought" goes into city management, most of the AI code is
 * actually in the City class.
 */
class PlayerAI
{
 public:
  PlayerAI(const Engine& engine);

  // Notify the AI player that the turn has cycled.
  void cycle_turn();

  // Getters

  unsigned tech_level() const { return m_tech_level; }

  unsigned tech_points() const { return m_tech_points; }

  unsigned next_tech_level_cost() const { return TECH_NEXT_LEVEL_COST_FUNC(m_tech_level); }

  unsigned population() const { return m_population; }

  // The AI receives yield bonuses based on their tech-level. This method
  // returns the tech-adjusted yield for a base yield.
  float get_adjusted_yield(float base_yield) const
  {
    return ADJUSTED_YIELD_FUNC(base_yield, m_tech_level);
  }

  xmlNodePtr to_xml();

 private:
  unsigned m_tech_level;
  unsigned m_tech_points;
  unsigned m_population;
  const Engine& m_engine;

  static constexpr unsigned STARTING_TECH_LEVEL   = 1;
  static constexpr unsigned FIRST_TECH_LEVEL_COST = 1000;

  static unsigned TECH_NEXT_LEVEL_COST_FUNC(unsigned tech_level)
  { return FIRST_TECH_LEVEL_COST * poly_growth(tech_level - STARTING_TECH_LEVEL, 1.5); }

  static unsigned TECH_POINT_FUNC(unsigned population)
  { return population / 100; }

  static unsigned ADJUSTED_YIELD_FUNC(float base_yield, unsigned tech_level)
  { return base_yield * (1 + ((tech_level - STARTING_TECH_LEVEL) * 0.1)); }

  // Forbidden
  PlayerAI(const PlayerAI&) = delete;
  PlayerAI& operator=(const PlayerAI&) = delete;
};

}

#endif

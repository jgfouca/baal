#ifndef PlayerAI_hpp
#define PlayerAI_hpp

#include "Drawable.hpp"

#include <libxml/parser.h>

namespace baal {

class City;

/**
 * Manager the "global" (higher-than-city) affairs of the AI. Since most
 * of the AI's "thought" goes into city management, most of the AI code is
 * actually in the City class.
 */
class PlayerAI : public Drawable
{
 public:
  PlayerAI();

  void cycle_turn();

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

  unsigned tech_level() const { return m_tech_level; }

  unsigned population() const { return m_population; }

  float tech_yield_multiplier() const
  { return 1 + (m_tech_level - STARTING_TECH_LEVEL) * YIELD_GAIN_PER_TECH; }

  xmlNodePtr to_xml();

 private:
  unsigned m_tech_level;
  unsigned m_tech_points;
  unsigned m_next_tech_level_cost;
  unsigned m_population;

  static const unsigned STARTING_TECH_LEVEL   = 1;
  static const unsigned FIRST_TECH_LEVEL_COST = 1000;
  static const float TECH_LEVEL_COST_INCREASE = 2.0;
  static const unsigned POP_PER_TECH_POINT    = 100;
  static const float YIELD_GAIN_PER_TECH      = 0.1;

  // Forbidden
  PlayerAI(const PlayerAI&);
  PlayerAI& operator=(const PlayerAI&);

};

}

#endif

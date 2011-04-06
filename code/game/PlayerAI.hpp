#ifndef PlayerAI_hpp
#define PlayerAI_hpp

#include "Drawable.hpp"

namespace baal {

class Engine;
class City;

class PlayerAI : public Drawable
{
 public:
  PlayerAI(Engine& engine);

  void cycle_turn();

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

  unsigned tech_level() const { return m_tech_level; }

 private:
  void manage_city(City& city);

  Engine&  m_engine;
  unsigned m_tech_level;
  unsigned m_tech_points;
  unsigned m_next_tech_level_cost;
  unsigned m_population;

  static const unsigned STARTING_TECH_LEVEL   = 1;
  static const unsigned FIRST_TECH_LEVEL_COST = 1000;
  static const float TECH_LEVEL_COST_INCREASE = 2.0;
  static const unsigned POP_PER_TECH_POINT    = 100;

  // Forbidden
  PlayerAI(const PlayerAI&);
  PlayerAI& operator=(const PlayerAI&);

};

}

#endif

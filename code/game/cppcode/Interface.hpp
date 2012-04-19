#ifndef Interface_hpp
#define Interface_hpp

#include <string>
#include <sstream>

#include "DrawMode.hpp"

namespace baal {

class Engine;
class Geology;
class Player;
class PlayerAI;
class Time;
class Atmosphere;
class Anomaly;
class World;
class WorldTile;

/**
 * Interfaces are responsible for presenting information to the
 * player and turning low-level player actions into official commands.
 * Interfaces create Command objects which apply themselves to
 * the game.
 */
class Interface
{
 public:
  Interface()
    : m_end_turns(0),
      m_draw_mode(CIV) // default to civ
  {}

  // Draw entire screen
  virtual void draw() = 0;

  // Draw individual items
  virtual void draw(const Geology&) = 0;
  virtual void draw(const Player&) = 0;
  virtual void draw(const PlayerAI&) = 0;
  virtual void draw(const Time&) = 0;
  virtual void draw(const Atmosphere&) = 0;
  virtual void draw(const Anomaly&) = 0;
  virtual void draw(const World&) = 0;
  virtual void draw(const WorldTile&) = 0;

  virtual void interact() = 0;

  virtual void help(const std::string& helpmsg) = 0;

  virtual void spell_report(const std::string& report) = 0;

  void end_turn(unsigned num_turns = 1) { m_end_turns = num_turns; }

  virtual void human_wins() = 0;

  virtual void ai_wins() = 0;

 protected:
  unsigned m_end_turns;
  DrawMode m_draw_mode;

 private:
  void set_draw_mode(DrawMode mode) { m_draw_mode = mode; }

  // only friends can change draw mode
  friend class DrawCommand;
};

#define SPELL_REPORT(msg)                                               \
do {                                                                    \
  std::ostringstream spell_report_oss;                                  \
  spell_report_oss << m_name << ": " << msg;                            \
  m_engine.interface().spell_report(spell_report_oss.str());            \
} while (false)

} // namespace baal

#endif

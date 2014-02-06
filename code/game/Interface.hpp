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
  Interface(unsigned tile_width, unsigned tile_height)
    : m_end_turns(0),
      m_draw_mode(CIV), // default to civ
      m_right_adjust(0),
      m_down_adjust(0),
      m_tile_width(tile_width),
      m_tile_height(tile_height)
  {}

  virtual ~Interface() {}

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

  // Correctness checks on adjustments will be done by the MoveCommand

  void adjust_left()
  { --m_right_adjust; }

  void adjust_right()
  { ++m_right_adjust; }

  void adjust_up()
  { --m_down_adjust; }

  void adjust_down()
  { ++m_down_adjust; }

  unsigned get_adjust_right() const
  { return m_right_adjust; }

  unsigned get_adjust_down() const
  { return m_down_adjust; }

  unsigned screen_tile_width() const
  { return m_tile_width; }

  unsigned screen_tile_height() const
  { return m_tile_height; }

 protected:
  unsigned m_end_turns;
  DrawMode m_draw_mode;
  unsigned m_right_adjust;
  unsigned m_down_adjust;
  unsigned m_tile_width;
  unsigned m_tile_height;

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

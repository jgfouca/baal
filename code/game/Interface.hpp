#ifndef Interface_hpp
#define Interface_hpp

#include <string>
#include <sstream>

namespace baal {

class Engine;

/**
 * Interfaces are responsible for presenting information to the
 * player and turning low-level player actions into official commands.
 * Interfaces create Command objects which apply themselves to
 * the game.
 */
class Interface
{
 public:
  Interface(Engine& engine)
    : m_engine(engine),
      m_end_turn(false)
  {}

  virtual void draw() = 0;

  virtual void interact() = 0;

  virtual void help(const std::string& helpmsg) = 0;

  virtual void spell_report(const std::string& report) = 0;

  void end_turn() { m_end_turn = true; }

 protected:
  Engine& m_engine;
  bool m_end_turn;

};

#define SPELL_REPORT(interface, msg) \
  do {                                                    \
    std::ostringstream spell_report_oss;                  \
    spell_report_oss << msg;                              \
    interface.spell_report(spell_report_oss.str());       \
  } while (false)

}

#endif

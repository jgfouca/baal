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
  Interface()
    : m_end_turn(false)
  {}

  virtual void draw() = 0;

  virtual void interact() = 0;

  virtual void help(const std::string& helpmsg) = 0;

  virtual void spell_report(const std::string& report) = 0;

  void end_turn() { m_end_turn = true; }

  virtual void human_wins() = 0;

  virtual void ai_wins() = 0;

 protected:
  bool m_end_turn;
};

#define SPELL_REPORT(msg)                                               \
do {                                                                    \
  std::ostringstream spell_report_oss;                                  \
  spell_report_oss << m_name << ": " << msg;                            \
  Engine::instance().interface().spell_report(spell_report_oss.str());  \
} while (false)

} // namespace baal

#endif

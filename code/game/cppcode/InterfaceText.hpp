#ifndef InterfaceText_hpp
#define InterfaceText_hpp

#include "Interface.hpp"

#include <iosfwd>

namespace baal {

class Engine;

/**
 * Text-based implementation of an interface
 */
class InterfaceText : public Interface
{
 public:
  InterfaceText(std::ostream& out, std::istream& in, Engine& engine);

  ~InterfaceText();

  virtual void draw();

  virtual void interact();

  virtual void help(const std::string& helpmsg);

  virtual void spell_report(const std::string& report);

  virtual void human_wins();

  virtual void ai_wins();

 private:
  std::ostream& m_ostream;
  std::istream& m_istream;
  Engine&       m_engine;
};

}

#endif

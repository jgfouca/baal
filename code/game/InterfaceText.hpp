#ifndef InterfaceText_hpp
#define InterfaceText_hpp

#include "Interface.hpp"

#include <iosfwd>

namespace baal {

/**
 * Text-based implementation of an interface
 */
class InterfaceText : public Interface
{
 public:
  InterfaceText(Engine& engine, std::ostream& out, std::istream& in);

  virtual void draw();

  virtual void interact();

  virtual void help(const std::string& helpmsg);

  virtual void spell_report(const std::string& report);

 private:
  std::ostream& m_ostream;
  std::istream& m_istream;
};

}

#endif

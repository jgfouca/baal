#ifndef InterfaceText_hpp
#define InterfaceText_hpp

#include "Interface.hpp"

#include <iosfwd>

namespace baal {

class InterfaceText : public Interface
{
 public:
  InterfaceText(std::ostream& stream) : m_stream(stream) {}
  
  virtual void draw(const World& world) const;

 private:
  std::ostream& m_stream;
};

}

#endif

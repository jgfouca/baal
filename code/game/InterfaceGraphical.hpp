#ifndef InterfaceGraphical_hpp
#define InterfaceGraphical_hpp

#include "Interface.hpp"
namespace baal {

class InterfaceGraphical : public Interface
{
 public:
  virtual void draw(const World& world) const { /*TODO*/ }

 private:
};

}

#endif

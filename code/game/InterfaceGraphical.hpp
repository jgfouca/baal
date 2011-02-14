#ifndef InterfaceGraphical_hpp
#define InterfaceGraphical_hpp

#include "Interface.hpp"
namespace baal {

class InterfaceGraphical : public Interface
{
 public:
  InterfaceGraphical(Engine& engine) : Interface(engine) {}

  virtual void draw() { /*TODO*/ }

  virtual void interact() { /*TODO*/ }

  virtual void help(const std::string& helpmsg) { /*TODO*/ }

 private:
};

}

#endif

#ifndef InterfaceGraphical_hpp
#define InterfaceGraphical_hpp

#ifndef NO_GRAPHICS

#include "Interface.hpp"

namespace baal {

class SGEGAMESTATE;

class InterfaceGraphical : public Interface
{
 public:
  static InterfaceGraphical* create(Engine& engine);

  static InterfaceGraphical* singleton();

  ~InterfaceGraphical();

  virtual void draw();

  void initEngine();

  void redraw(SGEGAMESTATE* state);

  virtual void interact();

  virtual void help(const std::string& helpmsg);

  void quit();

 private:
  InterfaceGraphical(Engine& engine);

  static InterfaceGraphical* INSTANCE;
};

}

#endif
#endif

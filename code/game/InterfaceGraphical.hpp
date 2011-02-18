#ifndef InterfaceGraphical_hpp
#define InterfaceGraphical_hpp

#include "Interface.hpp"
#include <sge.h>

namespace baal {

class InterfaceGraphical : public Interface
{
 public:
  static InterfaceGraphical* create(Engine& engine);

  static InterfaceGraphical* singleton();

 ~InterfaceGraphical();

  virtual void draw();

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

#ifndef InterfacePlayer_hpp
#define InterfacePlayer_hpp

#ifndef NO_GRAPHICS

#include "Player.hpp"
#include <SDL/sge.h>

namespace baal {

class InterfacePlayer
{
 public:

  InterfacePlayer(Player& player);

  void draw(SGEGAMESTATE* state);

  void init();

 private:

  SGESPRITE* m_sprite;

  SGEFONT* m_font;

  Player& m_player;
};

}

#endif
#endif

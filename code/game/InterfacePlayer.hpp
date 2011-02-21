#ifndef InterfacePlayer_hpp
#define InterfacePlayer_hpp

#include "Player.hpp"
#include <sge.h>

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

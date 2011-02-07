#ifndef Engine_hpp
#define Engine_hpp

namespace baal {

class World;
class Interface;
class Player;
class PlayerAI;

class Engine
{
 public:
  Engine();

  ~Engine();

  void play();

 private:
  const Interface& m_interface;
  World&           m_world;
  Player&          m_player;
  PlayerAI&        m_ai_player;
};

}

#endif

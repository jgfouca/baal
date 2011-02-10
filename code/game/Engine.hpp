#ifndef Engine_hpp
#define Engine_hpp

namespace baal {

class World;
class Interface;
class Player;
class PlayerAI;

/**
 * Engine will serve as a mediator between the other key classes. It also
 * facilitates interaction between objects by providing a way to get handles
 * to the other key objects. Finally, it manages the high-level flow of the
 * game.
 */
class Engine
{
 public:
  Engine();

  ~Engine();

  void play();

  World& world() { return m_world; }

  Interface& interface() { return m_interface; }

  void quit() { m_quit = true; }
  
 private:
  Interface& m_interface;
  World&     m_world;
  Player&    m_player;
  PlayerAI&  m_ai_player;
  bool       m_quit;
};

}

#endif

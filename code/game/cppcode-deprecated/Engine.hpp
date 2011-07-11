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
 *
 * Design: To avoid the tedium of passing this class around everywhere,
 * we've decided to make this class a singleton. The downside to this
 * choice is that it makes relationships between the classes poorly
 * defined since every class has access to each other.
 */
class Engine
{
 public:
  ~Engine();

  static Engine& instance();

  void play();

  World& world() { return m_world; }
  const World& world() const { return m_world; }

  Interface& interface() { return m_interface; }
  const Interface& interface() const { return m_interface; }

  Player& player() { return m_player; }
  const Player& player() const { return m_player; }

  PlayerAI& ai_player() { return m_ai_player; }
  const PlayerAI& ai_player() const { return m_ai_player; }

  void quit() { m_quit = true; }

 private:
  Engine();

  Interface& m_interface;
  World&     m_world;
  Player&    m_player;
  PlayerAI&  m_ai_player;
  bool       m_quit;

  // Forbidden
  Engine(const Engine&);
  Engine& operator=(const Engine&);
};

}

#endif

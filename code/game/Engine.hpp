#ifndef Engine_hpp
#define Engine_hpp

#include "Configuration.hpp"

namespace baal {

class World;
class Interface;
class Player;
class PlayerAI;
class Configuration;

/**
 * Engine will serve as a mediator between the other key classes. It also
 * facilitates interaction between objects by providing a way to get handles
 * to the other key objects. Finally, it manages the high-level flow of the
 * game.
 */
class Engine
{
 public:
  Engine(const Configuration& config = Configuration());

  ~Engine();

  void play();

  const Configuration& config() const { return m_config; }

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

  const Configuration& m_config;
  Interface&           m_interface;
  World&               m_world;
  Player&              m_player;
  PlayerAI&            m_ai_player;
  bool                 m_quit;

  // Forbidden
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
};

}

#endif

#ifndef Engine_hpp
#define Engine_hpp

#include "Configuration.hpp"

#include <memory>

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
  Engine(const Configuration& configuration);

  void init(std::shared_ptr<Interface> interface,
            std::shared_ptr<World> world,
            std::shared_ptr<Player> player,
            std::shared_ptr<PlayerAI> ai_player);

  void play();

  const Configuration& config() const { return m_config; }

  World& world() { return *m_world; }
  const World& world() const { return *m_world; }

  Interface& interface() { return *m_interface; }
  const Interface& interface() const { return *m_interface; }

  Player& player() { return *m_player; }
  const Player& player() const { return *m_player; }

  PlayerAI& ai_player() { return *m_ai_player; }
  const PlayerAI& ai_player() const { return *m_ai_player; }

  void quit();

 private:

  Configuration               m_config;
  bool                        m_quit;
  std::shared_ptr<Interface>  m_interface;
  std::shared_ptr<World>      m_world;
  std::shared_ptr<Player>     m_player;
  std::shared_ptr<PlayerAI>   m_ai_player;

  static constexpr unsigned AI_WINS_AT_TECH_LEVEL = 100;

  // Forbidden
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
};

// Factory function
std::shared_ptr<Engine> create_engine(const Configuration& config=Configuration());

}

#endif

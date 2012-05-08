#include "Engine.hpp"
#include "InterfaceFactory.hpp"
#include "WorldFactory.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"
#include "Interface.hpp"
#include "World.hpp"
#include "Configuration.hpp"

namespace baal {

///////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Engine> create_engine(const Configuration& config)
///////////////////////////////////////////////////////////////////////////////
{
  std::shared_ptr<Engine> rv(new Engine(config));

  std::shared_ptr<Interface> interface = InterfaceFactory::create(*rv);
  std::shared_ptr<World>     world     = WorldFactory::create(*rv);
  std::shared_ptr<Player>    player    (new Player(*rv));
  std::shared_ptr<PlayerAI>  ai_player (new PlayerAI(*rv));

  rv->init(interface, world, player, ai_player);

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
Engine::Engine(const Configuration& config)
///////////////////////////////////////////////////////////////////////////////
  : m_config(config),
    m_quit(false)
{}

///////////////////////////////////////////////////////////////////////////////
void Engine::init(std::shared_ptr<Interface> interface,
                  std::shared_ptr<World> world,
                  std::shared_ptr<Player> player,
                  std::shared_ptr<PlayerAI> ai_player)
///////////////////////////////////////////////////////////////////////////////
{
  m_interface = interface;
  m_world     = world;
  m_player    = player;
  m_ai_player = ai_player;
}

///////////////////////////////////////////////////////////////////////////////
void Engine::play()
///////////////////////////////////////////////////////////////////////////////
{
  // Game loop, each iteration of this loop is a full game turn
  while (!m_quit) {

    // Draw current game state
    m_interface->draw();

    // Human player takes turn
    m_interface->interact();
    m_player->cycle_turn();

    // AI player takes turn
    m_ai_player->cycle_turn();

    // Cycle world. Note this should always be the last item to cycle.
    m_world->cycle_turn();

    // Check for game-ending state
    if (m_ai_player->population() == 0) {
      m_interface->human_wins();
      break;
    }
    else if (m_ai_player->tech_level() >= AI_WINS_AT_TECH_LEVEL) {
      m_interface->ai_wins();
      break;
    }
  }
}

void Engine::quit()
{
  m_quit = true;
  m_interface->end_turn();
}

}

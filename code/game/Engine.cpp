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
Engine::Engine(const Configuration& config)
///////////////////////////////////////////////////////////////////////////////
  : m_config(config),
    m_interface(InterfaceFactory::create(*this)),
    m_world(WorldFactory::create(*this)),
    m_player(*(new Player(*this))), // might come from factory in the future
    m_ai_player(*(new PlayerAI(*this))), // might come from factory in the future
    m_quit(false)
{}

///////////////////////////////////////////////////////////////////////////////
Engine::~Engine()
///////////////////////////////////////////////////////////////////////////////
{
  delete &m_interface;
  delete &m_world;
  delete &m_player;
  delete &m_ai_player;
}

///////////////////////////////////////////////////////////////////////////////
void Engine::play()
///////////////////////////////////////////////////////////////////////////////
{
  // Game loop, each iteration of this loop is a full game turn
  while (!m_quit) {

    // Draw current game state
    m_interface.draw();

    // Human player takes turn
    m_interface.interact();
    m_player.cycle_turn();

    // AI player takes turn
    m_ai_player.cycle_turn();

    // Cycle world. Note this should always be the last item to cycle.
    m_world.cycle_turn();

    // Check for game-ending state
    if (m_ai_player.population() == 0) {
      m_interface.human_wins();
      break;
    }
    else if (m_ai_player.tech_level() >= AI_WINS_AT_TECH_LEVEL) {
      m_interface.ai_wins();
      break;
    }
  }
}

void Engine::quit()
{
  m_quit = true;
  interface().end_turn();
}

}

#include "Engine.hpp"
#include "InterfaceFactory.hpp"
#include "WorldFactory.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"
#include "Interface.hpp"
#include "World.hpp"
#include "Configuration.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
Engine::Engine()
///////////////////////////////////////////////////////////////////////////////
  : m_interface(InterfaceFactory::create()),
    m_world(WorldFactory::create()),
    m_player(*(new Player)), // might come from factory in the future
    m_ai_player(*(new PlayerAI)), // might come from factory in the future
    m_quit(false)
{
  // Configuration object should be initialized before this constructor is
  // called.
  Require(Configuration::instance().initialized(),
          "Out of order initialization");
}

///////////////////////////////////////////////////////////////////////////////
Engine& Engine::instance()
///////////////////////////////////////////////////////////////////////////////
{
#ifndef NDEBUG
  static unsigned call_count = 0;
  static bool init = false;
  ++call_count;
  Assert( !(call_count > 1 && !init),
          "Broken call stack: Engine instantiation has re-entered itself");
#endif

  static Engine engine;

#ifndef NDEBUG
  init = true;
#endif

  return engine;
}

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

    // Cycle world
    m_world.cycle_turn();

    // Check for game-ending state
    if (m_ai_player.population() == 0) {
      m_interface.human_wins();
      break;
    }
    else if (m_ai_player.tech_level() >= 100) {
      m_interface.ai_wins();
      break;
    }
  }
}

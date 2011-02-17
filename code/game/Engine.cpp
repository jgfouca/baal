#include "Engine.hpp"
#include "InterfaceFactory.hpp"
#include "WorldFactory.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"
#include "Interface.hpp"
#include "World.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
Engine::Engine()
///////////////////////////////////////////////////////////////////////////////
  : m_interface(InterfaceFactory::create(*this)),
    m_world(WorldFactory::create()),
    m_player(*(new Player)), // might come from factory in the future
    m_ai_player(*(new PlayerAI)), // might come from factory in the future
    m_quit(false)
{

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
    // TODO

    // Cycle world
    m_world.cycle_turn();
  }
}

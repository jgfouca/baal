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
  : m_interface(*(InterfaceFactory::create())),
    m_world(*(WorldFactory::create())),
    m_player(*(new Player)),
    m_ai_player(*(new PlayerAI))
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
  while (true) {
    // Draw current game state

    // Human player takes turn

    // AI player takes turn
    return; //TODO - fill in this loop
  }
}

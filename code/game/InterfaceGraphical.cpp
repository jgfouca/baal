#include "InterfaceGraphical.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "CommandFactory.hpp"
#include "Command.hpp"
#include "Player.hpp"

#include <sge.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>

using namespace baal;

extern "C" void doDraw(SGEGAMESTATE *state)
{
  InterfaceGraphical::singleton()->redraw(state);
}

extern "C" void launchGraphicThread()
{

        sgeInit(NOAUDIO,NOJOYSTICK);
        sgeOpenScreen("Baal",800,800,32, NOFULLSCREEN);
//      sgeHideMouse();

        SGEGAMESTATEMANAGER *manager;
        SGEGAMESTATE *mainstate;

        /*
         * create the main state and set the C (!) function stateMain
         * (from statemain.h/cpp) as onRedraw
         */
        mainstate = sgeGameStateNew();
        mainstate->onRedraw = &doDraw;

        // add the state to the state manager
        manager = sgeGameStateManagerNew();
        sgeGameStateManagerChange(manager, mainstate);

        // start the game manager on another thread
        sgeGameStateManagerRun(manager, 30);

       sgeCloseScreen();

       // tell the engine to quit
       InterfaceGraphical::singleton()->quit();
}

InterfaceGraphical* InterfaceGraphical::INSTANCE=NULL;

///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical::InterfaceGraphical
			    (Engine& engine)
///////////////////////////////////////////////////////////////////////////////
 : Interface(engine)
{
  boost::thread game_thread(launchGraphicThread);
}

///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical::~InterfaceGraphical()
///////////////////////////////////////////////////////////////////////////////
{
}


///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical* InterfaceGraphical::singleton()
///////////////////////////////////////////////////////////////////////////////
{
    //TODO: should this assert if create not called??
    return INSTANCE;
}

///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical* InterfaceGraphical::create(Engine& engine)
///////////////////////////////////////////////////////////////////////////////
{
  if (INSTANCE == NULL)
  {
    INSTANCE = new InterfaceGraphical(engine);
  }

  return INSTANCE;
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceGraphical::draw()
///////////////////////////////////////////////////////////////////////////////
{
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceGraphical::redraw(SGEGAMESTATE *state)
///////////////////////////////////////////////////////////////////////////////
{

    SGEEVENTSTATE es = state->manager->event_state;

        if (es.start.released) {
                sgeGameStateManagerQuit(state->manager);
                return;
        }

  SGEFILE* file=sgeOpenFile("../../data/data.d", "asdf");
  sgeLock(screen);  

	SGESPRITE* sprite= sgeSpriteNew();

  // Draw world
  const World& world = m_engine.world();
  for (unsigned row = 0; row < world.height(); ++row) {
    for (unsigned col = 0; col < world.width(); ++col) {
      switch (world.get_tile(Location(row, col)).type()) {
      case MTN:
        sgeSpriteAddFileRange(sprite, file, "grass%d.jpg", 1,2);
	sprite->x = 100 + row * 101;
        sprite->y = 100 + col * 101;
        sgeSpriteSetFPS(sprite, 1);
        sgeSpriteDraw(sprite, screen);
      //  m_ostream << "M ";
        break;
      case PLAIN:
    //    m_ostream << "P ";
        break;
      case OCEAN:
  //      m_ostream << "O ";
        break;
      case UNDEFINED:
        Require(false, "World[" << row << "][" << col << "] is undefined");
        break;
      default:
        Require(false, "Should never make it here");
      }
    }
//    m_ostream << "\n";
  }
//  m_ostream << "\n";

  // Draw Player
  const Player& player = m_engine.player();
 /* m_ostream << "PLAYER STATS:\n"
            << "  name: " << player.name() << "\n"
            << "  level: " << player.level() << "\n"
            << "  mana: " << player.mana() << "/" << player.max_mana() << "\n"
            << "  exp: " << player.exp() << "\n";
*/
  //m_ostream.flush();

  sgeUnlock(screen);
  sgeFlip();
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceGraphical::help(const std::string& helpmsg)
///////////////////////////////////////////////////////////////////////////////
{
  //m_ostream << helpmsg << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
void InterfaceGraphical::interact()
///////////////////////////////////////////////////////////////////////////////
{
	char choice;
	std::cin >> choice;
}

//////////////////////////////////////////////////////////////////////////////
void InterfaceGraphical::quit()
///////////////////////////////////////////////////////////////////////////////
{
  // Get handle to command factory
  const CommandFactory& cmd_factory = CommandFactory::instance();
  const Command& command = cmd_factory.parse_command("quit");
  command.apply(m_engine);
}

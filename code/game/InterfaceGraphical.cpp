#ifndef NO_GRAPHICS

#include "InterfaceGraphical.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "CommandFactory.hpp"
#include "Command.hpp"
#include "Player.hpp"

#include <SDL/sge.h>

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <string>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
extern "C" void doDraw(SGEGAMESTATE *state)
///////////////////////////////////////////////////////////////////////////////
{
  InterfaceGraphical::singleton()->draw(state);
}

///////////////////////////////////////////////////////////////////////////////
extern "C" void launchGraphicThread()
///////////////////////////////////////////////////////////////////////////////
{
  sgeInit(NOAUDIO, NOJOYSTICK);

  // now that the graphics are initialized...we can initalize the sprite information of the engine
  InterfaceGraphical::singleton()->init();

  sgeOpenScreen("Baal", 800, 800, 32, NOFULLSCREEN);

  SGEGAMESTATEMANAGER *manager;
  SGEGAMESTATE *mainstate;

  mainstate = sgeGameStateNew();
  mainstate->onRedraw = &doDraw;

  // add the state to the state manager
  manager = sgeGameStateManagerNew();
  sgeGameStateManagerChange(manager, mainstate);

  // Run the Manager loop
  sgeGameStateManagerRun(manager, 30);

  // we are done...close
  sgeCloseScreen();

  // tell the engine to quit
  InterfaceGraphical::singleton()->quit();
}

InterfaceGraphical* InterfaceGraphical::INSTANCE=NULL;

///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical::InterfaceGraphical(Engine& engine)
///////////////////////////////////////////////////////////////////////////////
 : Interface(engine),
   m_player_interface(*(new InterfacePlayer(engine.player())))
{
  // TODO: put thread call in the Factory
  // launch the new thread
   boost::thread game_thread(launchGraphicThread);

}

///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical::~InterfaceGraphical()
///////////////////////////////////////////////////////////////////////////////
{
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceGraphical::init()
///////////////////////////////////////////////////////////////////////////////
{
  SGEFILE *file = sgeOpenFile("../../data/data.d", "asdf");

  World& world = m_engine.world();
  for (unsigned row = 0; row < world.height(); ++row) {
    for (unsigned col = 0; col < world.width(); ++col) {
      // get the sprite information out of the data file
      // TODO: not hardcode data location?
      // create the sprite
      SGESPRITE* sprite = sgeSpriteNew();

      // load in correct sprite animation
      switch (world.get_tile(Location(row,col)).type())
      {
      case MTN:
           sgeSpriteAddFileRange(sprite, file, "tiles/mountain%d.jpg", 1, 2);
           break;
      case PLAIN:
           sgeSpriteAddFileRange(sprite, file, "tiles/plains%d.jpg", 1, 2);
           break;
      case OCEAN:
           sgeSpriteAddFileRange(sprite, file, "tiles/ocean%d.jpg", 1, 2);
           break;
      default:
        break;
      }

      // set the frame rate of the sprite animation
      sgeSpriteSetFPS(sprite, 1);

      world.get_tile(Location(row,col)).setSprite(sprite);
    }
  }

    //close the file
    sgeCloseFile(file);

    // now init the player
    m_player_interface.init();
}

///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical* InterfaceGraphical::singleton()
///////////////////////////////////////////////////////////////////////////////
{
  Require(INSTANCE != NULL,
          "Tried to use Graphical Interface before it was initalized");
  return INSTANCE;
}

///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical* InterfaceGraphical::create(Engine& engine)
///////////////////////////////////////////////////////////////////////////////
{
  if (INSTANCE == NULL) {
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
void InterfaceGraphical::draw(SGEGAMESTATE *state)
///////////////////////////////////////////////////////////////////////////////
{
  SGEEVENTSTATE es = state->manager->event_state;

  if (es.start.released) {
    sgeGameStateManagerQuit(state->manager);
    return;
  }

  sgeLock(screen);

  // Draw world
  const World& world = m_engine.world();
  for (unsigned row = 0; row < world.height(); ++row) {
    for (unsigned col = 0; col < world.width(); ++col) {

      SGESPRITE* sprite = world.get_tile(Location(row,col)).sprite();
      // sgeSpriteAddFileRange(sprite, file, "mountain%d.jpg", 1, 2);
      // if (sprite == NULL)
      //   continue;

        // TODO: not hardcode these numbers
        sprite->x = 175 + col * 101;
        sprite->y = 175 + row * 101;
        sgeSpriteDraw(sprite, screen);
    }
  }

  // Draw Player
  const Player& player = m_engine.player();
  // m_ostream << "PLAYER STATS:\n"
  //           << "  name: " << player.name() << "\n"
  //           << "  level: " << player.level() << "\n"
  //           << "  mana: " << player.mana() << "/" << player.max_mana() << "\n"
  //           << "  exp: " << player.exp() << "\n";
  // m_ostream.flush();

  sgeUnlock(screen);

  //TODO: pass screen around too?
  // now draw the player interface
  m_player_interface.draw(state);

  sgeFlip();
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceGraphical::help(const std::string& helpmsg)
///////////////////////////////////////////////////////////////////////////////
{
  // m_ostream << helpmsg << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
void InterfaceGraphical::interact()
///////////////////////////////////////////////////////////////////////////////
{
  while (!m_end_turn) {
  }
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

#endif

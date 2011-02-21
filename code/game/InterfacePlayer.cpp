#include "InterfacePlayer.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "CommandFactory.hpp"
#include "Command.hpp"
#include "Player.hpp"
#include "InterfaceConstants.hpp"

#include <sge.h>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
InterfacePlayer::InterfacePlayer
			    (Player& player)
: m_player(player)
///////////////////////////////////////////////////////////////////////////////
{
}

///////////////////////////////////////////////////////////////////////////////
void InterfacePlayer::init()
///////////////////////////////////////////////////////////////////////////////
{
  // TODO: doing always open the file but just keep pasing it around?

  // get the sprite information out of the data file
  // TODO not hardcode
  SGEFILE *file = sgeOpenFile("../../data/data.d", "asdf");
  m_sprite = sgeSpriteNew();

  // TODO: switch based on player name or identifier
  sgeSpriteAddFile(m_sprite, file, "player/baal.jpg");

  //TODO: clean up font at end sgeFontDestroy
  m_font = sgeFontNewFile(file, SGEFONT_BITMAP, "fonts/font.png");

  sgeCloseFile(file);
}

///////////////////////////////////////////////////////////////////////////////
void InterfacePlayer::draw(SGEGAMESTATE *state)
///////////////////////////////////////////////////////////////////////////////
{

  sgeLock(screen);

  const int fontHeight = sgeFontGetLineHeight(m_font); 

  // keep track of the current x,y to print stuff out
  int cur_x = PLAYER_INTERFACE_X;
  int cur_y = PLAYER_INTERFACE_Y;

  // Draw the player
  m_sprite->x = cur_x;
  m_sprite->y = cur_y;

  sgeSpriteDraw(m_sprite, screen);

  // translate down
  cur_y += PLAYER_PORTRAIT_HEIGHT + 4;

  // draw the player name
  sgeFontPrintBitmap(m_font, screen, cur_x, cur_y, m_player.name().c_str());

  // translate down
  // TODO: dont hardcode
  cur_y += fontHeight;
 
  // printout the level
  char * level = new char[m_player.level()/10 + 1];
  sprintf(level, "%s %d", "level", m_player.level());
  sgeFontPrint(m_font, screen, cur_x, cur_y, level);
  free(level);

  // translate down
  // TODO: dont hardcode
  cur_y += fontHeight;
 
  // printout the mana
  sgeFontPrintBitmap(m_font, screen, cur_x, cur_y, "2");

  // Draw Player
 /* m_ostream << "PLAYER STATS:\n"
            << "  name: " << player.name() << "\n"
            << "  level: " << player.level() << "\n"
            << "  mana: " << player.mana() << "/" << player.max_mana() << "\n"
            << "  exp: " << player.exp() << "\n";
*/
  sgeUnlock(screen);
}

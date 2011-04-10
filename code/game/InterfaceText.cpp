#include "InterfaceText.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "CommandFactory.hpp"
#include "Command.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"

#include <iostream>
#include <string>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
InterfaceText::InterfaceText(std::ostream& out,
                             std::istream& in)
///////////////////////////////////////////////////////////////////////////////
 : Interface(),
   m_ostream(out),
   m_istream(in)
{}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw()
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine = Engine::instance();

  // DESIGN: Should "drawable" items know how to draw themselves? That might
  // reduce coupling between those classes and the interface classes.

  clear_screen();

  // Draw world
  engine.world().draw_text(m_ostream);

  m_ostream << "\n";

  // Draw Player
  engine.player().draw_text(m_ostream);

  m_ostream << "\n";

  // Draw AI Player
  engine.ai_player().draw_text(m_ostream);

  m_ostream.flush();
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::help(const std::string& helpmsg)
///////////////////////////////////////////////////////////////////////////////
{
  m_ostream << helpmsg << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::spell_report(const std::string& report)
///////////////////////////////////////////////////////////////////////////////
{
  m_ostream << BOLD_COLOR << RED << "!! " << report << CLEAR_ALL << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::interact()
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine = Engine::instance();

  // Get handle to command factory
  const CommandFactory& cmd_factory = CommandFactory::instance();

  // Reset state
  m_end_turn = false;

  // Enter loop for this turn
  while(!m_end_turn) {
    // Grab a line of text
    m_ostream <<  "% ";
    m_ostream.flush();
    std::string command_str;
    if (!std::getline(m_istream, command_str)) {
      // User ctrl-d
      engine.quit();
      break;
    }
    else if (command_str.empty()) {
      continue;
    }

    try {
      const Command& command = cmd_factory.parse_command(command_str);
      command.apply();
    }
    catch (UserError& error) {
      m_ostream << "ERROR: " << error.what() << std::endl;
      m_ostream << "\nType: 'help [command]' for assistence" << std::endl;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::human_wins()
///////////////////////////////////////////////////////////////////////////////
{
  m_ostream << BOLD_COLOR << RED << "YOU'RE WINNAR!!" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::ai_wins()
///////////////////////////////////////////////////////////////////////////////
{
  m_ostream << BOLD_COLOR << RED << "YOU'RE LOZER!!" << std::endl;
}

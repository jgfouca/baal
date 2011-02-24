#include "InterfaceText.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "CommandFactory.hpp"
#include "Command.hpp"
#include "Player.hpp"

#include <iostream>
#include <string>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
InterfaceText::InterfaceText(Engine& engine,
                             std::ostream& out,
                             std::istream& in)
///////////////////////////////////////////////////////////////////////////////
 : Interface(engine),
   m_ostream(out),
   m_istream(in)
{}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw()
///////////////////////////////////////////////////////////////////////////////
{
  // DESIGN: Should "drawable" items know how to draw themselves? That might
  // reduce coupling between those classes and the interface classes.

  // Draw world
  m_engine.world().draw_text(m_ostream);

  m_ostream << "\n";

  // Draw Player
  m_engine.player().draw_text(m_ostream);

  m_ostream.flush();
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::help(const std::string& helpmsg)
///////////////////////////////////////////////////////////////////////////////
{
  m_ostream << helpmsg << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::interact()
///////////////////////////////////////////////////////////////////////////////
{
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
      // Empty commands are not acceptable
      m_ostream << "please enter command" << std::endl;
      continue;
    }

    try {
      const Command& command = cmd_factory.parse_command(command_str);
      command.apply(m_engine);
    }
    catch (UserError& error) {
      m_ostream << "ERROR: " << error.what() << std::endl;
      m_ostream << "\nType: 'help [command]' for assistence" << std::endl;
    }
  }
}

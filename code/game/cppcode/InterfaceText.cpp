#include "InterfaceText.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "CommandFactory.hpp"
#include "Command.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"
#include "Util.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <readline/readline.h>
#include <readline/history.h>
#include <boost/algorithm/string.hpp>

namespace baal {

///////////////////////////////////////////////////////////////////////////////
InterfaceText::InterfaceText(std::ostream& out,
                             std::istream& in,
                             Engine& engine)
///////////////////////////////////////////////////////////////////////////////
 : Interface(),
   m_ostream(out),
   m_istream(in),
   m_engine(engine)
{
  initialize_readline();
}

///////////////////////////////////////////////////////////////////////////////
InterfaceText::~InterfaceText()
///////////////////////////////////////////////////////////////////////////////
{
  std::ofstream* outfile = dynamic_cast<std::ofstream*>(&m_ostream);
  std::ifstream* infile  = dynamic_cast<std::ifstream*>(&m_istream);

  if (outfile != nullptr) {
    outfile->close();
    delete &outfile;
  }

  if (infile != nullptr) {
    infile->close();
    delete &infile;
  }
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw()
///////////////////////////////////////////////////////////////////////////////
{
  // DESIGN: Should "drawable" items know how to draw themselves? That might
  // reduce coupling between those classes and the interface classes.

  clear_screen();

  // Draw world
  m_engine.world().draw_text(m_ostream);

  m_ostream << "\n";

  // Draw Player
  m_engine.player().draw_text(m_ostream);

  m_ostream << "\n";

  // Draw AI Player
  m_engine.ai_player().draw_text(m_ostream);

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
  // Get handle to command factory
  const CommandFactory& cmd_factory = CommandFactory::instance();

  // readline library is in C - here comes the diarrhea
  char *line = nullptr;

  // Enter loop for this turn
  while(m_end_turns == 0) {
    // Grab a line of text
    line = readline("% ");
    if (line == nullptr){
      // User ctrl-d
      m_engine.quit();
      break;
    }

    // Add to history and process if not empty string
    if (std::strlen(line) > 0) {
      add_history(line);
      std::string command_str(line);
      boost::trim(command_str);
      try {
        std::shared_ptr<const Command> command =
          cmd_factory.parse_command(command_str, m_engine);
        command->apply();
      }
      catch (UserError& error) {
        m_ostream << "ERROR: " << error.what() << std::endl;
        m_ostream << "\nType: 'help [command]' for assistence" << std::endl;
      }
    }
    free(line);
  }

  --m_end_turns;
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

}

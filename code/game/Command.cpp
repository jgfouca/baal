#include "Command.hpp"
#include "CommandFactory.hpp"
#include "Engine.hpp"
#include "BaalExceptions.hpp"
#include "Interface.hpp"

#include <ctime>
#include <sstream>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
void HelpCommand::init(const std::vector<std::string>& args)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() <= 1, "The help command takes at most one argument");

  if (!args.empty()) {
    m_arg = args.front();

    // Verify m_arg is a valid command name
    const CommandFactory& factory = CommandFactory::instance();
    RequireUser(factory.m_cmd_map.find(m_arg) != factory.m_cmd_map.end(),
                "Cannot get help for unknown command " << m_arg);
  }
  else {
    m_arg = "";
  }
}

///////////////////////////////////////////////////////////////////////////////
void HelpCommand::apply(Engine& engine) const
///////////////////////////////////////////////////////////////////////////////
{
  const CommandFactory& factory = CommandFactory::instance();

  if (m_arg.empty()) {
    std::string help_msg = "List of available commands:\n\n";
    for (std::map<std::string, Command*>::const_iterator
         itr = factory.m_cmd_map.begin();
         itr != factory.m_cmd_map.end();
         ++itr) {
      help_msg += itr->second->help() + "\n\n";
    }
    engine.interface().help(help_msg);
  }
  else {
    engine.interface().help(factory.m_cmd_map.find(m_arg)->second->help());
  }
}

///////////////////////////////////////////////////////////////////////////////
std::string HelpCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return std::string(
"help [command]\n"
"  Returns info/syntax help for a command"
                     );
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void EndTurnCommand::init(const std::vector<std::string>& args)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.empty(), "The end command takes no arguments");
}

///////////////////////////////////////////////////////////////////////////////
void EndTurnCommand::apply(Engine& engine) const
///////////////////////////////////////////////////////////////////////////////
{
  engine.interface().end_turn();
}

///////////////////////////////////////////////////////////////////////////////
std::string EndTurnCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return std::string(
"end\n"
"  Ends the current turn"
                     );
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void QuitCommand::init(const std::vector<std::string>& args)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.empty(), "The quit command takes no arguments");
}

///////////////////////////////////////////////////////////////////////////////
void QuitCommand::apply(Engine& engine) const
///////////////////////////////////////////////////////////////////////////////
{
  engine.interface().end_turn();
  engine.quit();
}

///////////////////////////////////////////////////////////////////////////////
std::string QuitCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return std::string(
"quit\n"
"  Ends the game"
                     );
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void SaveCommand::init(const std::vector<std::string>& args)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() <= 1, "The save command takes at most one argument");

  if (!args.empty()) {
    m_arg = args.front();
  }
  else {
    // Create a save name based on time/date

    std::ostringstream out;

    // Get current time and convert to tm struct
    time_t curr_time_out = time(NULL);
    struct tm* UTC_time_out = localtime(&curr_time_out);

    // Create save name
    out << "baal_"
        << UTC_time_out->tm_year + 1900 << "-"
        << UTC_time_out->tm_mon + 1 << "-"
        << UTC_time_out->tm_mday << "__"
        << UTC_time_out->tm_hour << ":"
        << UTC_time_out->tm_min << ":"
        << UTC_time_out->tm_sec
        << ".save";

    m_arg = out.str();
  }
}

///////////////////////////////////////////////////////////////////////////////
void SaveCommand::apply(Engine& engine) const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO
}

///////////////////////////////////////////////////////////////////////////////
std::string SaveCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return std::string(
"save [save filename]\n"
"  Saves the game; if no name provided, a name based on data/time will be used"
                     );
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void SpellCommand::init(const std::vector<std::string>& args)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() == 3, "The cast command takes 3 arguments");

  // Parse spell name
  m_spell_name = args[0];
  // TODO: Require valid spell name

  // Parse spell level
  std::istringstream iss(args[1]);
  iss >> m_spell_level;
  RequireUser(!iss.fail(), "Second argument not a valid integer");

  // Parse location
  try {
    m_spell_location = Location(args[2]);
  }
  catch (ProgramError& e) {
    RequireUser(false, "Third argument was not a valid location. " <<
                "Expect <row>,<col> (no spaces)\nError was: " << e.what());
  }
}

///////////////////////////////////////////////////////////////////////////////
void SpellCommand::apply(Engine& engine) const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO
}

///////////////////////////////////////////////////////////////////////////////
std::string SpellCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return std::string(
"cast <spell-name> <level> <row>,<col>\n"
"  Casts spell of type <spell-name> and level <level> at location <row>,<col>"
                     );
}

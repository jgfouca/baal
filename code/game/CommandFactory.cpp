#include "CommandFactory.hpp"
#include "Command.hpp"
#include "BaalExceptions.hpp"

#include <sstream>
#include <vector>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
const CommandFactory& CommandFactory::instance()
///////////////////////////////////////////////////////////////////////////////
{
  static CommandFactory global_cmd_factory;
  return global_cmd_factory;
}

///////////////////////////////////////////////////////////////////////////////
CommandFactory::CommandFactory()
///////////////////////////////////////////////////////////////////////////////
{
  // Populate command map, this is the only place where the list of all
  // commands is exposed

  m_cmd_map["help"] = new HelpCommand;
  m_cmd_map["save"] = new SaveCommand;
  m_cmd_map["end" ] = new EndTurnCommand;
  m_cmd_map["quit"] = new QuitCommand;
  m_cmd_map["cast"] = new SpellCommand;
}

///////////////////////////////////////////////////////////////////////////////
CommandFactory::~CommandFactory()
///////////////////////////////////////////////////////////////////////////////
{
  for (std::map<std::string, Command*>::iterator
       itr = m_cmd_map.begin(); itr != m_cmd_map.end(); ++itr) {
    delete itr->second;
  }
}

///////////////////////////////////////////////////////////////////////////////
const Command& CommandFactory::parse_command(const std::string& text) const
///////////////////////////////////////////////////////////////////////////////
{
  std::istringstream iss(text);

  // Get first token of command
  std::string cmd_name;
  iss >> cmd_name;
  RequireUser(!iss.fail(), "Failed while retrieving command name (first token)");

  // Get command args
  std::vector<std::string> args;
  while (!iss.eof()) {
    std::string token;
    iss >> token;
    RequireUser(!iss.fail(), "Failed while retrieving token " << args.size());
    args.push_back(token);
  }

  // Init and return command object
  std::map<std::string, Command*>::const_iterator itr = m_cmd_map.find(cmd_name);
  RequireUser(itr != m_cmd_map.end(),
              "Unknown command: " << cmd_name << ". Type 'help' for help.");
  Command& command = *(itr->second);
  command.init(args);
  return command;
}

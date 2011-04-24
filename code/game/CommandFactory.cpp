#include "CommandFactory.hpp"
#include "Command.hpp"
#include "BaalExceptions.hpp"

#include <sstream>
#include <vector>

using namespace baal;
#include <iostream>
using namespace std;

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

  // TODO:
  // This implementation (recycling/reinitializing heap objects) is
  // error-prone and should probably be replace with a system similar to that
  // of SpellFactory.
  m_cmd_map["help" ] = new HelpCommand;
  m_cmd_map["save" ] = new SaveCommand;
  m_cmd_map["end"  ] = new EndTurnCommand;
  m_cmd_map["quit" ] = new QuitCommand;
  m_cmd_map["cast" ] = new SpellCommand;
  m_cmd_map["learn"] = new LearnCommand;
  m_cmd_map["draw" ] = new DrawCommand;
  m_cmd_map["hack" ] = new HackCommand;

  // Set up aliases
  m_aliases["s"] = "save";
  m_aliases["n"] = "end";
  m_aliases["q"] = "quit";
  m_aliases["c"] = "cast";
  m_aliases["l"] = "learn";
  m_aliases["d"] = "draw";
  m_aliases["h"] = "hack";
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
const std::map<std::string, Command*>* const CommandFactory::getCommandMap() const
{
  const std::map<std::string, Command*>* const mapPtr = &m_cmd_map;
  return mapPtr;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const Command& CommandFactory::parse_command(const std::string& text) const
///////////////////////////////////////////////////////////////////////////////
{
  std::istringstream iss(text);

  // Get first token of command
  std::string cmd_name;
  iss >> cmd_name;
  RequireUser(!iss.fail(),
              "Failed while retrieving command name (first token)");

  // Get command args
  std::vector<std::string> args;
  while (!iss.eof()) {
    std::string token;
    iss >> token;
    RequireUser(!iss.fail(), "Failed while retrieving token " << args.size());
    args.push_back(token);
  }

  // Init and return command object. Note that command objects are recycled.
  // I would have much rather stored a map to Command classes, but that's not
  // possible in C++. The command map is necessary for the HelpCommand.
  std::map<std::string, Command*>::const_iterator itr = m_cmd_map.find(cmd_name);
  if (itr == m_cmd_map.end()) {
    std::map<std::string, std::string>::const_iterator alias_itr = m_aliases.find(cmd_name);
    RequireUser(alias_itr != m_aliases.end(),
                "Unknown command: " << cmd_name << ". Type 'help' for help.");
    itr = m_cmd_map.find(alias_itr->second);
    Require(itr != m_cmd_map.end(), "Broken alias: " << alias_itr->second);
  }

  Command& command = *(itr->second);
  command.init(args);
  return command;
}

///////////////////////////////////////////////////////////////////////////////
const std::string& CommandFactory::name(const Command* command) const
///////////////////////////////////////////////////////////////////////////////
{
  for (std::map<std::string, Command*>::const_iterator
       itr = m_cmd_map.begin(); itr != m_cmd_map.end(); ++itr) {
    if (itr->second == command) {
      return itr->first;
    }
  }
  Require(false, "Never found command");
}

///////////////////////////////////////////////////////////////////////////////
void CommandFactory::aliases(const std::string& name,
                             std::vector<std::string>& alias_rv) const
///////////////////////////////////////////////////////////////////////////////
{
  for (std::map<std::string, std::string>::const_iterator
       itr = m_aliases.begin(); itr != m_aliases.end(); ++itr) {
    if (itr->second == name) {
      alias_rv.push_back(itr->first);
    }
  }
}

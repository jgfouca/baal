#include "CommandFactory.hpp"
#include "Command.hpp"
#include "BaalExceptions.hpp"

#include <sstream>
#include <vector>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
const Command* CommandFactory::parse_command(const std::string& text)
///////////////////////////////////////////////////////////////////////////////
{
  std::istringstream iss(text);

  // Get first token of command
  std::string command_name;
  iss >> command_name;
  RequireUser(!iss.fail(), "Failed while retrieving command name (first token)");

  // Get command args
  std::vector<std::string> args;
  while (!iss.eof()) {
    std::string token;
    iss >> token;
    RequireUser(!iss.fail(), "Failed while retrieving token " << args.size());
    args.push_back(token);
  }

  // Create command object
  Command* rv;
  if (command_name == "help") {
    rv = new HelpCommand(args);
  }
  else if (command_name == "save") {
    rv = new SaveCommand(args);
  }
  else if (command_name == "end") {
    rv = new EndTurnCommand(args);
  }
  else if (command_name == "quit") {
    rv = new QuitCommand(args);
  }
  else if (command_name == "cast") {
    rv = new SpellCommand(args);
  }
  else {
    RequireUser(false, "Unknown command: " << command_name);
  }

  return rv;
}

#include "Command.hpp"
#include "Engine.hpp"
#include "BaalExceptions.hpp"
#include "Interface.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
HelpCommand::HelpCommand(const std::vector<std::string>& args)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() <= 1, "The help command takes at most one argument");

  if (!args.empty()) {
    m_arg = args.front();
    // TODO - Verify m_arg is a valid command name
  }
}

///////////////////////////////////////////////////////////////////////////////
void HelpCommand::apply(Engine& engine) const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO - Fill in. How to limit knowledge of Command "library"?
  if (m_arg.empty()) {
    engine.interface().help("General help");
  }
  else {
    engine.interface().help(std::string("Help for command ")  + m_arg);
  }
}

///////////////////////////////////////////////////////////////////////////////
EndTurnCommand::EndTurnCommand(const std::vector<std::string>& args)
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
QuitCommand::QuitCommand(const std::vector<std::string>& args)
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

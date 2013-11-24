#include "CommandFactory.hpp"
#include "Command.hpp"
#include "BaalExceptions.hpp"

#include <sstream>
#include <vector>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>

#include <boost/type_traits/detail/wrap.hpp>

using namespace boost::mpl::placeholders;
namespace mpl = boost::mpl;
namespace traits = boost::type_traits;

namespace baal {

namespace {

///////////////////////////////////////////////////////////////////////////////
struct SearchAndCreate
///////////////////////////////////////////////////////////////////////////////
{
  SearchAndCreate(const std::string& name,
                  const vecstr_t& args,
                  Command*& return_val,
                  Engine& engine) :
    m_name(name),
    m_args(args),
    m_return_val(return_val),
    m_engine(engine)
  {}

  template <class CommandClass>
  void operator()(traits::wrap<CommandClass>)
  {
    if (m_name == CommandClass::NAME) {
      Require(m_return_val == nullptr, "Found multiple matches");
      m_return_val = new CommandClass(m_args, m_engine);
    }
  }

  const std::string& m_name;
  const vecstr_t& m_args;
  Command*& m_return_val;
  Engine& m_engine;
};

}

namespace command_factory_only {

///////////////////////////////////////////////////////////////////////////////
struct Initializer
///////////////////////////////////////////////////////////////////////////////
{
  Initializer(CommandFactory& factory) : m_factory(factory) {}

  template <class CommandClass>
  void operator()(traits::wrap<CommandClass>)
  {
    std::map<std::string, std::string>& aliases = m_factory.m_aliases;
    vecstr_t& commands = m_factory.m_commands;

    Require(!contains(commands, CommandClass::NAME),
            "Duplicate command name " << CommandClass::NAME);

    commands.push_back(CommandClass::NAME);

    for (std::string alias : CommandClass::ALIASES) {
      Require(!contains(aliases, alias), "Duplicate alias: " << alias);
      Require(!contains(commands, alias),
              "Alias " << alias << " conflicts with command name");
      aliases[alias] = CommandClass::NAME;
    }
  }

  CommandFactory& m_factory;
};

}

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
  m_commands.reserve(mpl::size<command_types>::value);
  mpl::for_each<
    mpl::transform<command_types, traits::wrap<_1> >::type
    > (command_factory_only::Initializer(*this));
}

///////////////////////////////////////////////////////////////////////////////
std::shared_ptr<const Command>
CommandFactory::parse_command(const std::string& text, Engine& engine) const
///////////////////////////////////////////////////////////////////////////////
{
  std::istringstream iss(text);

  // Get first token of command
  std::string cmd_name;
  iss >> cmd_name;
  RequireUser(!iss.fail(),
              "Failed while retrieving command name (first token)");

  // Get command args
  vecstr_t args;
  while (!iss.eof()) {
    std::string token;
    iss >> token;
    RequireUser(!iss.fail(), "Failed while retrieving token " << args.size());
    args.push_back(token);
  }

  // If alias, translate to real name
  {
    std::map<std::string, std::string>::const_iterator itr = m_aliases.find(cmd_name);
    if (itr != m_aliases.end()) {
      cmd_name = itr->second;
    }
  }

  // Search for type and create appropriate Command object
  Command* new_cmd = nullptr;
  SearchAndCreate search_and_create_functor(cmd_name,
                                            args,
                                            new_cmd,
                                            engine);
  mpl::for_each<
    mpl::transform<command_types, traits::wrap<_1> >::type
    >(search_and_create_functor);
  RequireUser(new_cmd != nullptr,
              "Unknown command: " << cmd_name << ". Type 'help' for help.");

  return std::shared_ptr<const Command>(new_cmd);
}

///////////////////////////////////////////////////////////////////////////////
void CommandFactory::aliases(const std::string& name,
                             vecstr_t& alias_rv) const
///////////////////////////////////////////////////////////////////////////////
{
  for (std::map<std::string, std::string>::const_iterator
       itr = m_aliases.begin(); itr != m_aliases.end(); ++itr) {
    if (itr->second == name) {
      alias_rv.push_back(itr->first);
    }
  }
}

}

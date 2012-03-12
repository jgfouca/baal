#include "CommandFactory.hpp"
#include "Command.hpp"
#include "BaalExceptions.hpp"

#include <sstream>
#include <vector>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>

namespace baal {

namespace {

struct SearchAndCreate
{
  SearchAndCreate(const std::string& name,
                  const std::vector<std::string>& args,
                  Command*& return_val,
                  Engine& engine) :
    m_name(name),
    m_args(args),
    m_return_val(return_val),
    m_engine(engine)
  {}

  template <class CommandClass>
  void operator()(CommandClass)
  {
    if (m_name == CommandClass::NAME) {
      Require(m_return_val == nullptr, "Found multiple matches");
      m_return_val = new CommandClass(m_args, m_engine);
    }
  }

  const std::string& m_name;
  const std::vector<std::string>& m_args;
  Command*& m_return_val;
  Engine& m_engine;
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
  m_cmd_map.reserve(boost::mpl::size<command_types>::value);
  boost::mpl::for_each<command_types>(Initializer(*this));
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
  std::vector<std::string> args;
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
  boost::mpl::for_each<command_types>(search_and_create_functor);
  RequireUser(new_cmd != nullptr,
              "Unknown command: " << cmd_name << ". Type 'help' for help.");

  return std::shared_ptr<const Command>(new_cmd);
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

}

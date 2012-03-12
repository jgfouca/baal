#ifndef CommandFactory_hpp
#define CommandFactory_hpp

#include "Command.hpp"
#include "BaalExceptions.hpp"

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <boost/mpl/vector.hpp>

namespace baal {

class Engine;

/**
 * A CommandFactory creates commands. It encapsulates the knowledge of the
 * set of available commands. This class is a singleton.
 *
 * Design: The singleton pattern here is harmless because CommandFactory
 * instances are all the same.
 */
class CommandFactory
{
 public:
  static const CommandFactory& instance();

  std::shared_ptr<const Command> parse_command(const std::string& text,
                                               Engine& engine) const;

  void aliases(const std::string& name, std::vector<std::string>& alias_rv) const;

  const std::vector<std::string>& get_command_map() const { return m_cmd_map; }

  typedef boost::mpl::vector<HelpCommand,
                             SaveCommand,
                             EndTurnCommand,
                             QuitCommand,
                             SpellCommand,
                             LearnCommand,
                             DrawCommand,
                             HackCommand> command_types;

 private:
  // Private constructor since this is singleton class
  CommandFactory();

  // Disallowed methods
  CommandFactory(const CommandFactory&) = delete;
  CommandFactory& operator=(const CommandFactory&) = delete;

  // Members
  std::vector<std::string>           m_cmd_map;
  std::map<std::string, std::string> m_aliases;

  struct Initializer
  {
    Initializer(CommandFactory& factory) : m_factory(factory) {}

    template <class CommandClass>
    void operator()(CommandClass)
    {
      std::map<std::string, std::string>& aliases = m_factory.m_aliases;
      std::vector<std::string>& commands = m_factory.m_cmd_map;

      Require(std::find(commands.begin(), commands.end(), CommandClass::NAME) ==
              commands.end(),
              "Duplicate command name " << CommandClass::NAME);

      m_factory.m_cmd_map.push_back(CommandClass::NAME);

      for (std::string alias : CommandClass::ALIASES) {
        Require(aliases.find(alias) == aliases.end(),
                "Duplicate alias: " << alias);
        Require(std::find(commands.begin(), commands.end(), alias) ==
                commands.end(),
                "Alias " << alias << " conflicts with command name");
        aliases[alias] = CommandClass::NAME;
      }
    }

    CommandFactory& m_factory;
  };
};

}

#endif

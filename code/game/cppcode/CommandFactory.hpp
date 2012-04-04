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

namespace command_factory_only {
struct Initializer;
}

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

  friend struct command_factory_only::Initializer;
};

}

#endif

#ifndef CommandFactory_hpp
#define CommandFactory_hpp

#include <string>
#include <map>

namespace baal {

class Command;

/**
 * A CommandFactory creates commands. It encapsulates the knowledge of the
 * different commands, IE it encapsulates the fact that there are many
 * different types of commands. This class is a singleton.
 */
class CommandFactory
{
 public:
  static const CommandFactory& instance();

  ~CommandFactory();

  // Does not need to be deleted
  const Command& parse_command(const std::string& text) const;

 private:
  // Private constructor since this is singleton class
  CommandFactory();

  // Disallowed methods
  CommandFactory(const CommandFactory&);
  CommandFactory& operator=(const CommandFactory&);

  // Members
  std::map<std::string, Command*> m_cmd_map;

  // Friends
  friend class HelpCommand;
};

}

#endif

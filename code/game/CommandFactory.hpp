#ifndef CommandFactory_hpp
#define CommandFactory_hpp

#include <string>
#include <map>
#include <vector>

namespace baal {

class Command;

/**
 * A CommandFactory creates commands. It encapsulates the knowledge of the
 * set of available commands. This class is a singleton.
 */
class CommandFactory
{
 public:
  static const CommandFactory& instance();

  ~CommandFactory();

  // Does not need to be deleted
  const Command& parse_command(const std::string& text) const;

  const std::string& name(const Command* command) const;

  void aliases(const std::string& name, std::vector<std::string>& alias_rv) const;

  const std::map<std::string, Command*> * const getCommandMap() const;

 private:
  // Private constructor since this is singleton class
  CommandFactory();

  // Disallowed methods
  CommandFactory(const CommandFactory&);
  CommandFactory& operator=(const CommandFactory&);

  // Members
  std::map<std::string, Command*> m_cmd_map;
  std::map<std::string, std::string> m_aliases;

  // Friends
  friend class HelpCommand;
};

}

#endif

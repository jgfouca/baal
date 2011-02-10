#ifndef CommandFactory_hpp
#define CommandFactory_hpp

#include <string>

namespace baal {

class Command;

/**
 * A CommandFactory creates commands. It encapsulates the knowledge of the
 * different commands.
 */
class CommandFactory
{
 public:
  // Needs to be deleted by client
  static const Command* parse_command(const std::string& text);
};

}

#endif

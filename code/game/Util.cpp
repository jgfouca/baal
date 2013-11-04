#include "Util.hpp"
#include "CommandFactory.hpp"
#include "Configuration.hpp"
#include "Engine.hpp"

#include <cstring>
#include <cstdlib>

#include <readline/readline.h>
#include <readline/history.h>

namespace baal {

namespace {

///////////////////////////////////////////////////////////////////////////////
char* dupstr(char *s)
///////////////////////////////////////////////////////////////////////////////
{
  char* r = (char *) std::malloc(strlen(s) + 1);
  strcpy(r, s);
  return r;
}

///////////////////////////////////////////////////////////////////////////////
char* command_generator (const char* text, int state)
///////////////////////////////////////////////////////////////////////////////
{
  static int len;
  static vecstr_t::const_iterator list_index;
  char *name;

  // Get handle to command factory, and pointer to command map
  const CommandFactory& cmd_factory = CommandFactory::instance();
  const vecstr_t& cmd_map = cmd_factory.commands();

  if (!state) {
    list_index = cmd_map.begin();
    len = std::strlen(text);
  }

  while ( list_index != cmd_map.end() ) {
    name = (char *)(list_index->c_str());
    ++list_index;

    if (std::strncmp(name, text, len) == 0){
      return dupstr(name);
    }
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
char** baal_completion(const char* text, int start, int end)
///////////////////////////////////////////////////////////////////////////////
{
  if (start == 0) {
    return rl_completion_matches(text, command_generator);
  }

  return nullptr;
}

} // empty namespace

///////////////////////////////////////////////////////////////////////////////
void initialize_readline ()
///////////////////////////////////////////////////////////////////////////////
{
  rl_readline_name = (char*) "Baal";

  // readline binding defines autocomplete behavior
  rl_attempted_completion_function = baal_completion;
}

} // namespace baal

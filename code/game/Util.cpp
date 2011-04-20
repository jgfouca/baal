#include "Util.hpp"
#include "Interface.hpp"
#include "InterfaceText.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "CommandFactory.hpp"
#include "Command.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"

#include <readline/readline.h>
#include <readline/history.h>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
void initialize_readline ()
///////////////////////////////////////////////////////////////////////////////
{
  rl_readline_name = "Baal";
  rl_attempted_completion_function = baal_completion;
}

///////////////////////////////////////////////////////////////////////////////
char * dupstr (char *s)
///////////////////////////////////////////////////////////////////////////////
{
  char *r;

  r = (char *) malloc (strlen (s) + 1);
  strcpy (r, s);
  return (r);
}

///////////////////////////////////////////////////////////////////////////////
char ** baal_completion (const char *text, int start, int end)
///////////////////////////////////////////////////////////////////////////////
{
  char **matches;
  matches = (char **)NULL;

  if (start == 0)
    matches = rl_completion_matches (text, command_generator);

  return (matches);
}

///////////////////////////////////////////////////////////////////////////////
char * command_generator (const char *text, int state)
///////////////////////////////////////////////////////////////////////////////
{
  static int len;
  static std::map<std::string, Command*>::const_iterator list_index;
  char *name;

  // Get handle to command factory, and pointer to command map
  const CommandFactory& cmd_factory = CommandFactory::instance();
  const std::map<std::string, Command*>* const cmd_map = cmd_factory.getCommandMap();

  if (!state)
    {
      list_index = cmd_map->begin();
      len = strlen (text);
    }

  while ( list_index != cmd_map->end() )
    {
      name = (char *)(list_index->first.c_str());
      ++list_index;

      if (strncmp (name, text, len) == 0){
        return (dupstr(name));
      }
    }

  return ((char *)NULL);
}


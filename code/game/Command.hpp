#ifndef Command_hpp
#define Command_hpp

#include <string>
#include <vector>

#include "BaalCommon.hpp"

// We use this class to define all the commands. This will avoid
// creation of lots of very small hpp/cpp files.

namespace baal {

class Engine;

/**
 * A Command is the entity by which a player affects the game state.
 * Command apply themselves immediately upon creation.
 */
class Command
{
 public:
  virtual void apply(Engine& engine) const = 0;
};

/**
 * How the player asks for help.
 *
 * Syntax: help [command]
 */
class HelpCommand : public Command
{
 public:
  HelpCommand(const std::vector<std::string>& args);

  virtual void apply(Engine& engine) const;

 private:
  std::string m_arg;
};

/**
 * Save the current game
 *
 * Syntax: save [filename]
 */
class SaveCommand : public Command
{
 public:
  SaveCommand(const std::vector<std::string>& args) { /*TODO*/ }

  virtual void apply(Engine& engine) const { /*TODO*/ }
 
 private:
  std::string m_arg;
};

/**
 * End the current turn.
 *
 * Syntax: end
 */
class EndTurnCommand : public Command
{
 public:
  EndTurnCommand(const std::vector<std::string>& args);

  virtual void apply(Engine& engine) const;
};

/**
 * Quit the game.
 *
 * Syntax: quit
 */
class QuitCommand : public Command
{
 public:
  QuitCommand(const std::vector<std::string>& args);

  virtual void apply(Engine& engine) const;
};

/**
 * Cast a spell
 *
 * Syntax: cast <spell-name> <row>,<col> <level>
 */
class SpellCommand : public Command
{
 public:
  SpellCommand(const std::vector<std::string>& args) : m_spell_location(0, 0) { /*TODO*/ }

  virtual void apply(Engine& engine) const { /*TODO*/ }

 private:
  std::string m_spell_name;
  Location    m_spell_location;
  unsigned    m_spell_level;
};

}

#endif

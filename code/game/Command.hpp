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
  virtual void init(const std::vector<std::string>& args) = 0;

  virtual void apply(Engine& engine) const = 0;

  virtual std::string help() const = 0;
};

/**
 * How the player asks for help.
 *
 * Syntax: help [command]
 */
class HelpCommand : public Command
{
 public:
  virtual void init(const std::vector<std::string>& args);

  virtual void apply(Engine& engine) const;

  virtual std::string help() const;

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
  virtual void init(const std::vector<std::string>& args);

  virtual void apply(Engine& engine) const;

  virtual std::string help() const;

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
  virtual void init(const std::vector<std::string>& args);

  virtual void apply(Engine& engine) const;

  virtual std::string help() const;
};

/**
 * Quit the game.
 *
 * Syntax: quit
 */
class QuitCommand : public Command
{
 public:
  virtual void init(const std::vector<std::string>& args);

  virtual void apply(Engine& engine) const;

  virtual std::string help() const;
};

/**
 * Cast a spell
 *
 * Syntax: cast <spell-name> <level> <row>,<col>
 */
class SpellCommand : public Command
{
 public:
  virtual void init(const std::vector<std::string>& args);

  virtual void apply(Engine& engine) const;

  virtual std::string help() const;

 private:
  std::string m_spell_name;
  unsigned    m_spell_level;
  Location    m_spell_location;
};

}

#endif

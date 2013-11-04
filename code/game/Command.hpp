#ifndef Command_hpp
#define Command_hpp

#include <string>
#include <vector>

#include "BaalCommon.hpp"

// We use this file to define all the commands. This will avoid
// creation of lots of very small hpp/cpp files.

namespace baal {

class Engine;

/**
 * A Command is the entity by which a player affects the game state.
 */
class Command
{
 public:
  Command(Engine& engine) : m_engine(engine) {}

  virtual ~Command() {}

  virtual void apply() const = 0;

 protected:
  Command() : m_engine(*static_cast<Engine*>(nullptr)) {}

  Engine& m_engine;
};

/**
 * How the player asks for help.
 *
 * Syntax: help [item]
 */
class HelpCommand : public Command
{
 public:
  HelpCommand(const vecstr_t& args, Engine& engine);

  virtual void apply() const;

  static const std::string NAME;

  static const std::string HELP;

  static const vecstr_t ALIASES;
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
  SaveCommand(const vecstr_t& args, Engine& engine);

  virtual void apply() const;

  static const std::string NAME;

  static const std::string HELP;

  static const vecstr_t ALIASES;
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
  EndTurnCommand(const vecstr_t& args, Engine& engine);

  virtual void apply() const;

  static const std::string NAME;

  static const std::string HELP;

  static const vecstr_t ALIASES;
 private:
  unsigned m_num_turns;

  static const unsigned MAX_SKIP_TURNS = 100;
};

/**
 * Quit the game.
 *
 * Syntax: quit
 */
class QuitCommand : public Command
{
 public:
  QuitCommand(const vecstr_t& args, Engine& engine);

  virtual void apply() const;

  static const std::string NAME;

  static const std::string HELP;

  static const vecstr_t ALIASES;
};

/**
 * Cast a spell
 *
 * Syntax: cast <spell-name> <level> <row>,<col>
 */
class SpellCommand : public Command
{
 public:
  SpellCommand(const vecstr_t& args, Engine& engine);

  virtual void apply() const;

  static const std::string NAME;

  static const std::string HELP;

  static const vecstr_t ALIASES;
 private:
  std::string m_spell_name;
  Location    m_spell_location;
  unsigned    m_spell_level;
};

/**
 * Learn a spell
 *
 * Syntax: learn <spell-name> <level>
 */
class LearnCommand : public Command
{
 public:
  LearnCommand(const vecstr_t& args, Engine& engine);

  virtual void apply() const;

  static const std::string NAME;

  static const std::string HELP;

  static const vecstr_t ALIASES;
 private:
  std::string m_spell_name;
};

/**
 * Change draw mode
 *
 * Syntax: draw <mode>
 */
class DrawCommand : public Command
{
 public:
  DrawCommand(const vecstr_t& args, Engine& engine);

  virtual void apply() const;

  static const std::string NAME;

  static const std::string HELP;

  static const vecstr_t ALIASES;
 private:
  std::string m_draw_mode;
};

/**
 * Allow user to give exp to player. Be sure to hide this in the
 * release version. This is only for debugging purposes.
 */
class HackCommand : public Command
{
 public:
  HackCommand(const vecstr_t& args, Engine& engine);

  virtual void apply() const;

  static const std::string NAME;

  static const std::string HELP;

  static const vecstr_t ALIASES;
 private:
  unsigned m_exp;
};

}

#endif

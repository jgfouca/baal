#include "Command.hpp"
#include "CommandFactory.hpp"
#include "Engine.hpp"
#include "BaalExceptions.hpp"
#include "Interface.hpp"
#include "Spell.hpp"
#include "SpellFactory.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "Drawable.hpp"

#include <ctime>
#include <sstream>
#include <memory>

#include <boost/mpl/for_each.hpp>

namespace baal {

const std::string HelpCommand::NAME    = "help";
const std::string SaveCommand::NAME    = "save";
const std::string EndTurnCommand::NAME = "end";
const std::string QuitCommand::NAME    = "quit";
const std::string SpellCommand::NAME   = "cast";
const std::string LearnCommand::NAME   = "learn";
const std::string DrawCommand::NAME    = "draw";
const std::string HackCommand::NAME    = "hack";

const std::vector<std::string> HelpCommand::ALIASES    = {"h"};
const std::vector<std::string> SaveCommand::ALIASES    = {"s"};
const std::vector<std::string> EndTurnCommand::ALIASES = {"n"};
const std::vector<std::string> QuitCommand::ALIASES    = {"q"};
const std::vector<std::string> SpellCommand::ALIASES   = {"c"};
const std::vector<std::string> LearnCommand::ALIASES   = {"l"};
const std::vector<std::string> DrawCommand::ALIASES    = {"d"};
const std::vector<std::string> HackCommand::ALIASES    = {"x"};

const std::string HelpCommand::HELP =
  "[command]\n"
  "  Returns info/syntax help for a command or all commands if no argument";
const std::string SaveCommand::HELP =
  "[filename]\n"
  "  Saves the game; if no name provided, a name based on data/time will be used";
const std::string EndTurnCommand::HELP =
  "[num-turns]\n"
  "  Ends the current turn. Optional arg to skip ahead many turns";
const std::string QuitCommand::HELP =
  "\n"
  "  Ends the game\n";
const std::string SpellCommand::HELP =
  "<spell-name> <level> <row>,<col>\n"
  "  Casts spell of type <spell-name> and level <level> at location <row>,<col>";
const std::string LearnCommand::HELP =
  "<spell-name> <level>\n"
  "  Player learns spell of type <spell-name> and level <level>";
const std::string DrawCommand::HELP =
  "<draw-mode>\n"
  "  Changes how the world is drawn.";
const std::string HackCommand::HELP =
  "<exp>\n"
  "  Gives the player free arbitrary exp. This is a cheat put in for testing";

namespace {

///////////////////////////////////////////////////////////////////////////////
template <class CommandClass>
std::string create_help_str()
///////////////////////////////////////////////////////////////////////////////
{
  // Transform aliases into string
  std::string alias_str;
  for (const std::string& alias : CommandClass::ALIASES) {
    alias_str += alias + " ";
  }

  // Formulate and return full help string
  std::string help_str = CommandClass::NAME + " " + CommandClass::HELP;
  if (alias_str !=  "") {
    help_str += "  Aliases: " + alias_str;
  }
  return help_str;
}

///////////////////////////////////////////////////////////////////////////////
struct CreateHelpDump
///////////////////////////////////////////////////////////////////////////////
{
  CreateHelpDump(const std::string& name,
                 std::ostringstream& out,
                 Engine& engine) :
    m_name(name),
    m_return_val(out),
    m_engine(engine)
  {}

  template <class CommandClass>
  void operator()(CommandClass)
  {
    if (m_name == "" || m_name == CommandClass::NAME) {
      m_return_val << create_help_str<CommandClass>() << "\n";
    }
  }

  void operator()(SpellCommand)
  {
    if (m_name == "" || m_name == SpellCommand::NAME) {
      m_return_val << create_help_str<SpellCommand>() << "\n";

      const TalentTree& talents = m_engine.player().talents();

      std::vector<std::pair<std::string, unsigned> > castable_spells;
      talents.query_all_castable_spells(castable_spells);

      m_return_val << "  Castable spells:\n";
      for (const std::pair<std::string, unsigned>& spell_spec : castable_spells) {
        std::string spell_name = spell_spec.first;
        unsigned spell_lvl     = spell_spec.second;
        m_return_val << "    " << spell_name << " : " << spell_lvl << "\n";
      }
    }
  }

  void operator()(LearnCommand)
  {
    if (m_name == "" || m_name == LearnCommand::NAME) {
      m_return_val << create_help_str<LearnCommand>() << "\n";

      Player& player = m_engine.player();
      const TalentTree& talents = player.talents();

      std::vector<std::pair<std::string, unsigned> > learnable_spells;
      talents.query_all_learnable_spells(learnable_spells, player);

      m_return_val << "  Learnable spells:\n";
      for (const std::pair<std::string, unsigned>& spell_spec : learnable_spells) {
        std::string spell_name = spell_spec.first;
        unsigned spell_lvl     = spell_spec.second;
        m_return_val << "    " << spell_name << " : " << spell_lvl << "\n";
      }

    }
  }

  void operator()(DrawCommand)
  {
    if (m_name == "" || m_name == DrawCommand::NAME) {
      m_return_val << create_help_str<DrawCommand>() << "\n";

      m_return_val << "  Available draw modes:\n";
      for (DrawMode mode_itr = Drawable::FIRST; ; ++mode_itr) {
        m_return_val << "    " << Drawable::draw_mode_to_str(mode_itr) << "\n";
        if (mode_itr == Drawable::LAST) {
          break;
        }
      }
    }
  }

  const std::string& m_name;
  std::ostringstream& m_return_val;
  Engine& m_engine;
};

} // empty namespace

///////////////////////////////////////////////////////////////////////////////
HelpCommand::HelpCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine),
  m_arg("")
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() <= 1, "The help command takes at most one argument");

  if (!args.empty()) {
    m_arg = args.front();

    // Verify m_arg is a valid command name
    const CommandFactory& factory = CommandFactory::instance();
    const std::vector<std::string>& cmd_map = factory.get_command_map();
    RequireUser(std::find(cmd_map.begin(), cmd_map.end(), m_arg) != cmd_map.end(),
                "Cannot get help for unknown command " << m_arg);
  }
}

///////////////////////////////////////////////////////////////////////////////
void HelpCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  std::ostringstream out;
  CreateHelpDump help_dump_functor(m_arg, out, m_engine);
  if (m_arg == "") {
    std::string help_msg = "List of available commands:\n\n" + out.str();
    m_engine.interface().help(help_msg);
  }
  else {
    m_engine.interface().help(out.str());
  }
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
EndTurnCommand::EndTurnCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine),
  m_num_turns(1)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() <= 1, "The end command takes at most one argument");

  if (!args.empty()) {
    // Parse num turns
    std::istringstream iss(args[0]);
    iss >> m_num_turns;
    RequireUser(!iss.fail(), "Argument not a valid integer");

    RequireUser(m_num_turns > 0 && m_num_turns <= MAX_SKIP_TURNS,
                "num-turns must be between 0 and " << MAX_SKIP_TURNS);
  }
}

///////////////////////////////////////////////////////////////////////////////
void EndTurnCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  m_engine.interface().end_turn(m_num_turns);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
QuitCommand::QuitCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.empty(), "The quit command takes no arguments");
}

///////////////////////////////////////////////////////////////////////////////
void QuitCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  m_engine.interface().end_turn();
  m_engine.quit();
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
SaveCommand::SaveCommand(const std::vector<std::string>& args, Engine& engine)
  : Command(engine)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() <= 1, "The save command takes at most one argument");

  if (!args.empty()) {
    m_arg = args.front();
  }
  else {
    // Create a save name based on time/date

    std::ostringstream out;

    // Get current time and convert to tm struct
    time_t curr_time_out = time(nullptr);
    struct tm* UTC_time_out = localtime(&curr_time_out);

    // Create save name
    out << "baal_"
        << UTC_time_out->tm_year + 1900 << "-"
        << UTC_time_out->tm_mon + 1 << "-"
        << UTC_time_out->tm_mday << "__"
        << UTC_time_out->tm_hour << ":"
        << UTC_time_out->tm_min << ":"
        << UTC_time_out->tm_sec
        << ".save";

    m_arg = out.str();
  }
}

///////////////////////////////////////////////////////////////////////////////
void SaveCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // IN PROGRESS
  World&  world  = m_engine.world();
  Player& player = m_engine.player();
  //PlayerAI& ai_player = m_engine.ai_player();

  xmlDocPtr doc = nullptr;
  xmlNodePtr root_node = nullptr;
  doc = xmlNewDoc(BAD_CAST "1.0");
  root_node = xmlNewNode(nullptr, BAD_CAST "baal_root");
  xmlDocSetRootElement(doc, root_node);

  xmlAddChild(root_node, world.to_xml());
  xmlAddChild(root_node, player.to_xml());
  //xmlAddChild(root_node, ai_player.to_xml());

  xmlSaveFormatFileEnc(m_arg.c_str(), doc, "UTF-8", 1);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
SpellCommand::SpellCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() == 3, "The cast command takes 3 arguments");

  // Parse spell name
  m_spell_name = args[0];

  // Parse spell level
  std::istringstream iss(args[1]);
  iss >> m_spell_level;
  RequireUser(!iss.fail(), "Second argument not a valid integer");

  // Parse location
  try {
    m_spell_location = Location(args[2]);
  }
  catch (ProgramError& e) {
    RequireUser(false, "Third argument was not a valid location. " <<
                "Expect <row>,<col> (no spaces)\nError was: " << e.what());
  }
}

///////////////////////////////////////////////////////////////////////////////
void SpellCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  World&  world  = m_engine.world();
  Player& player = m_engine.player();

  // Ensure location is in-bounds
  RequireUser(world.in_bounds(m_spell_location),
              "Location " << m_spell_location << " out of bounds. " <<
              "Max row is: " << world.height() - 1 <<
              ", max col is: " << world.width() - 1);

  // Create the spell. I'd rather use a reference here since spell
  // cannot be nullptr, but we need to use a shared-ptr since verify_cast can
  // throw exceptions.
  std::shared_ptr<const Spell> spell(
    &(SpellFactory::create_spell(m_spell_name,
                                 m_spell_level,
                                 m_spell_location,
                                 m_engine))
                                   );

  // Verify that player can cast this spell (can throw)
  player.verify_cast(*spell);

  // Verify that it makes sense to cast this exact spell (can throw)
  spell->verify_apply();

  // These last two operations need to be atomic, neither should ever throw
  // a user error.
  try {
    // Let the player object know that the spell has been cast and to adjust
    // it's state accordingly.
    player.cast(*spell);

    // Apply the spell to the world
    unsigned exp = spell->apply();

    // Give player experience
    player.gain_exp(exp);
  }
  catch (UserError& error) {
    Require(false, "User error interrupted atomic operations...\n" <<
            "Error: " << error.what());
  }
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
LearnCommand::LearnCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() == 2, "The learn command takes 2 arguments");

  // Parse spell name
  m_spell_name = args[0];

  // Parse spell level
  std::istringstream iss(args[1]);
  iss >> m_spell_level;
  RequireUser(!iss.fail(), "Second argument not a valid integer");
}

///////////////////////////////////////////////////////////////////////////////
void LearnCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Player& player = m_engine.player();
  Location dummy;

  // We need to auto_ptr since learn can throw exceptions.
  std::auto_ptr<const Spell> spell(
    &(SpellFactory::create_spell(m_spell_name,
                                 m_spell_level,
                                 dummy,
                                 m_engine))
                                   );

  // Try to have the player learn this spell, this can throw
  player.learn(*spell);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
DrawCommand::DrawCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() == 1, "The draw command takes 1 argument");

  // Parse draw mode
  m_draw_mode = args[0];
}

///////////////////////////////////////////////////////////////////////////////
void DrawCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  DrawMode new_draw_mode = Drawable::parse_draw_mode(m_draw_mode);
  Drawable::set_draw_mode(new_draw_mode);
  m_engine.interface().draw(); // redraw
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
HackCommand::HackCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() == 1, "The hack command takes 1 argument");

  // Parse exp
  std::istringstream iss(args[0]);
  iss >> m_exp;
  RequireUser(!iss.fail(), "Argument not a valid integer");
}

///////////////////////////////////////////////////////////////////////////////
void HackCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Player& player = m_engine.player();
  player.gain_exp(m_exp);
}

}

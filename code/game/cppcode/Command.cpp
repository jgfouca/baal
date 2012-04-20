#include "Command.hpp"
#include "CommandFactory.hpp"
#include "Engine.hpp"
#include "BaalExceptions.hpp"
#include "Interface.hpp"
#include "Spell.hpp"
#include "SpellFactory.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "DrawMode.hpp"

#include <ctime>
#include <sstream>
#include <memory>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>

#include <boost/type_traits/detail/wrap.hpp>

using namespace boost::mpl::placeholders;
namespace mpl = boost::mpl;
namespace traits = boost::type_traits;

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
  "[item]\n"
  "  Returns info/syntax help for an item or all commands if no argument\n"
  "  Examples of valid items: command-name, spell-name, mode-name";
const std::string SaveCommand::HELP =
  "[filename]\n"
  "  Saves the game; if no name provided, a name based on data/time will be used";
const std::string EndTurnCommand::HELP =
  "[num-turns]\n"
  "  Ends the current turn. Optional arg to skip ahead many turns";
const std::string QuitCommand::HELP =
  "\n"
  "  Ends the game";
const std::string SpellCommand::HELP =
  "<spell-name> <row>,<col> [<level>]\n"
  "  Casts spell of type <spell-name> and level <level> at location <row>,<col>\n"
  "  If no level is provided, spell will be cast at player's max skill";
const std::string LearnCommand::HELP =
  "<spell-name>\n"
  "  Player learns spell of type <spell-name> or increases power in that spell";
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
    help_str += "\n  Aliases: " + alias_str;
  }
  return help_str;
}

///////////////////////////////////////////////////////////////////////////////
struct CreateHelpDump
///////////////////////////////////////////////////////////////////////////////
{
  /////////////////////////////////////////////////////////////////////////////
  CreateHelpDump(const std::string& name,
                 std::ostringstream& out,
                 Engine& engine) :
  /////////////////////////////////////////////////////////////////////////////
    m_name(name),
    m_return_val(out),
    m_engine(engine)
  {}

  /////////////////////////////////////////////////////////////////////////////
  template <class CommandClass>
  void operator()(traits::wrap<CommandClass>)
  /////////////////////////////////////////////////////////////////////////////
  {
    if (m_name == "" || m_name == CommandClass::NAME) {
      m_return_val << create_help_str<CommandClass>() << "\n";
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  void operator()(traits::wrap<SpellCommand>)
  /////////////////////////////////////////////////////////////////////////////
  {
    if (m_name == "" || m_name == SpellCommand::NAME) {
      m_return_val << create_help_str<SpellCommand>() << "\n";

      const TalentTree& talents = m_engine.player().talents();

      auto castable_spells = talents.query_all_castable_spells();

      m_return_val << "  Castable spells:\n";
      for (const std::pair<std::string, unsigned>& spell_spec : castable_spells) {
        std::string spell_name = spell_spec.first;
        unsigned spell_lvl     = spell_spec.second;
        m_return_val << "    " << spell_name << " : " << spell_lvl << "\n";
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  void operator()(traits::wrap<LearnCommand>)
  /////////////////////////////////////////////////////////////////////////////
  {
    if (m_name == "" || m_name == LearnCommand::NAME) {
      m_return_val << create_help_str<LearnCommand>() << "\n";

      Player& player = m_engine.player();
      const TalentTree& talents = player.talents();

      auto learnable_spells= talents.query_all_learnable_spells();

      m_return_val << "  Learnable spells:\n";
      for (const std::pair<std::string, unsigned>& spell_spec : learnable_spells) {
        std::string spell_name = spell_spec.first;
        unsigned spell_lvl     = spell_spec.second;
        std::ostringstream out;
        if (spell_lvl == 1) {
          out << "(new)";
        }
        else {
          out << spell_lvl - 1;
        }
        m_return_val << "    " << spell_name << " : " << out.str() << "\n";
      }

    }
  }

  /////////////////////////////////////////////////////////////////////////////
  void operator()(traits::wrap<DrawCommand>)
  /////////////////////////////////////////////////////////////////////////////
  {
    if (m_name == "" || m_name == DrawCommand::NAME) {
      m_return_val << create_help_str<DrawCommand>() << "\n";

      m_return_val << "  Available draw modes:\n";
      for (DrawMode mode_itr = FIRST; ; ++mode_itr) {
        m_return_val << "    " << draw_mode_to_str(mode_itr) << "\n";
        if (mode_itr == LAST) {
          break;
        }
      }
    }
  }

  const std::string& m_name;
  std::ostringstream& m_return_val;
  Engine& m_engine;
};

///////////////////////////////////////////////////////////////////////////////
template <class T>
struct is_in_command_factory :
  mpl::contains<CommandFactory::command_types, T>
{};
///////////////////////////////////////////////////////////////////////////////

} // empty namespace

///////////////////////////////////////////////////////////////////////////////
HelpCommand::HelpCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine),
  m_arg("")
///////////////////////////////////////////////////////////////////////////////
{
  BOOST_MPL_ASSERT(( is_in_command_factory<HelpCommand> ));
  RequireUser(args.size() <= 1,
              "'" << HelpCommand::NAME << "' takes at most one argument");

  if (!args.empty()) {
    m_arg = args.front();
  }
}

///////////////////////////////////////////////////////////////////////////////
void HelpCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  std::ostringstream out;

  // Case 1: argument is empty or refers to command
  if (m_arg == "" ||
      contains(m_arg, CommandFactory::instance().get_command_map())) {
    if (m_arg == "") {
      out << "List of available commands:\n\n";
    }
    CreateHelpDump help_dump_functor(m_arg, out, m_engine);
    mpl::for_each<
      mpl::transform<CommandFactory::command_types,
                     traits::wrap<_1> >::type
      >(help_dump_functor);
  }
  // Case 2: argument is a spell name
  else if (SpellFactory::is_in_all_names(m_arg)) {
    const Spell& spell = SpellFactory::create_spell(m_arg, m_engine);

    out << "Description of " << m_arg << " spell:\n"
        << spell.info() << "\n"
        << "Player has skill level "
        << m_engine.player().talents().spell_skill(m_arg)
        << " in this spell";
    delete &spell;
  }
  // Case 3: argument is a draw mode
  else {
    try {
      DrawMode mode = parse_draw_mode(m_arg);
      out << "Description of draw-mode: " << m_arg << "\n"
          << explain_draw_mode(mode);
    }
    catch (const UserError& e) {
      // Case 4: no match
      RequireUser(false, "Unrecognized item: " << m_arg);
    }
  }

  m_engine.interface().help(out.str());
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
EndTurnCommand::EndTurnCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine),
  m_num_turns(1)
///////////////////////////////////////////////////////////////////////////////
{
  BOOST_MPL_ASSERT(( is_in_command_factory<EndTurnCommand> ));
  RequireUser(args.size() <= 1,
              "'" << EndTurnCommand::NAME << "' takes at most one argument");

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
  BOOST_MPL_ASSERT(( is_in_command_factory<QuitCommand> ));
  RequireUser(args.empty(),
              "'" << QuitCommand::NAME << "' takes no arguments");
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
  BOOST_MPL_ASSERT(( is_in_command_factory<SaveCommand> ));
  RequireUser(args.size() <= 1,
              "'" << SaveCommand::NAME << "' takes at most one argument");

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
  BOOST_MPL_ASSERT(( is_in_command_factory<SpellCommand> ));
  RequireUser(args.size() >= 2 && args.size() <= 3,
              "'" << SpellCommand::NAME << "' takes two or three arguments");

  // Parse spell name
  m_spell_name = args[0];

  // Parse location
  try {
    m_spell_location = Location(args[1]);
  }
  catch (ProgramError& e) {
    RequireUser(false, "Second argument was not a valid location. " <<
                "Expect <row>,<col> (no spaces)\nError was: " << e.what());
  }

  // Parse spell level
  if (args.size() == 3) {
    std::istringstream iss(args[2]);
    iss >> m_spell_level;
    RequireUser(!iss.fail(), "Third argument not a valid integer");
  }
  else {
    m_spell_level = m_engine.player().talents().spell_skill(m_spell_name);
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
                                 m_engine,
                                 m_spell_level,
                                 m_spell_location)));

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
  BOOST_MPL_ASSERT(( is_in_command_factory<LearnCommand> ));
  RequireUser(args.size() == 1,
              "'" << LearnCommand::NAME << "' takes one argument");

  // Parse spell name
  m_spell_name = args[0];
}

///////////////////////////////////////////////////////////////////////////////
void LearnCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // Try to have the player learn this spell, this can throw
  m_engine.player().learn(m_spell_name);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
DrawCommand::DrawCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine)
///////////////////////////////////////////////////////////////////////////////
{
  BOOST_MPL_ASSERT(( is_in_command_factory<DrawCommand> ));
  RequireUser(args.size() == 1,
              "'" << DrawCommand::NAME << "' takes one argument");

  // Parse draw mode
  m_draw_mode = args[0];
}

///////////////////////////////////////////////////////////////////////////////
void DrawCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  DrawMode new_draw_mode = parse_draw_mode(m_draw_mode);
  m_engine.interface().set_draw_mode(new_draw_mode);
  m_engine.interface().draw(); // redraw
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
HackCommand::HackCommand(const std::vector<std::string>& args, Engine& engine) :
  Command(engine),
  m_exp(0)
///////////////////////////////////////////////////////////////////////////////
{
  BOOST_MPL_ASSERT(( is_in_command_factory<HackCommand> ));
  RequireUser(args.size() <= 1,
              "'" << HackCommand::NAME << "' takes at most one argument");

  // Parse exp
  if (args.size() == 1) {
    std::istringstream iss(args[0]);
    iss >> m_exp;
    RequireUser(!iss.fail(), "Argument not a valid integer");
  }
}

///////////////////////////////////////////////////////////////////////////////
void HackCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Player& player = m_engine.player();
  if (m_exp) {
    player.gain_exp(m_exp);
  }
  else {
    player.gain_exp(player.next_level_cost() - player.exp());
  }
}

}

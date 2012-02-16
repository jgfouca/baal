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

namespace baal {

namespace {

///////////////////////////////////////////////////////////////////////////////
std::string create_help_str(const Command* command,
                            const std::string& usage)
///////////////////////////////////////////////////////////////////////////////
{
  const CommandFactory& factory = CommandFactory::instance();

  // Get command name
  std::string command_name = factory.name(command);

  // Get command aliases
  std::vector<std::string> aliases;
  factory.aliases(command_name, aliases);

  // Transform aliases into string
  std::string alias_str;
  if (!aliases.empty()) {
    for (std::vector<std::string>::const_iterator
         itr = aliases.begin(); itr != aliases.end(); ++itr) {
      alias_str += *itr + " ";
    }
  }

  // Formulate and return full help string
  std::string help_str = command_name + " " + usage;
  if (!aliases.empty()) {
    help_str += "  Aliases: " + alias_str;
  }
  return help_str;
}

} // empty namespace

///////////////////////////////////////////////////////////////////////////////
void HelpCommand::init(const std::vector<std::string>& args)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() <= 1, "The help command takes at most one argument");

  if (!args.empty()) {
    m_arg = args.front();

    // Verify m_arg is a valid command name
    const CommandFactory& factory = CommandFactory::instance();
    RequireUser(factory.m_cmd_map.find(m_arg) != factory.m_cmd_map.end(),
                "Cannot get help for unknown command " << m_arg);
  }
  else {
    m_arg = "";
  }
}

///////////////////////////////////////////////////////////////////////////////
void HelpCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine = Engine::instance();
  const std::map<std::string, Command*>& cmd_map =
    CommandFactory::instance().m_cmd_map;

  if (m_arg.empty()) {
    std::string help_msg = "List of available commands:\n\n";
    for (std::map<std::string, Command*>::const_iterator
         itr = cmd_map.begin();
         itr != cmd_map.end();
         ++itr) {
      help_msg += itr->second->help() + "\n\n";
    }
    engine.interface().help(help_msg);
  }
  else {
    std::map<std::string, Command*>::const_iterator
      itr = cmd_map.find(m_arg);
    Require(itr != cmd_map.end(), "Command " << m_arg << " missing from map");
    engine.interface().help(itr->second->help());
  }
}

///////////////////////////////////////////////////////////////////////////////
std::string HelpCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return create_help_str(this,
"[command]\n"
"  Returns info/syntax help for a command or all commands if no argument\n"
                         );
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void EndTurnCommand::init(const std::vector<std::string>& args)
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
  else {
    m_num_turns = 1;
  }
}

///////////////////////////////////////////////////////////////////////////////
void EndTurnCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Engine::instance().interface().end_turn(m_num_turns);
}

///////////////////////////////////////////////////////////////////////////////
std::string EndTurnCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return create_help_str(this,
"[num-turns]\n"
"  Ends the current turn. Optional arg to skip ahead many turns\n"
                         );
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void QuitCommand::init(const std::vector<std::string>& args)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.empty(), "The quit command takes no arguments");
}

///////////////////////////////////////////////////////////////////////////////
void QuitCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine = Engine::instance();
  engine.interface().end_turn();
  engine.quit();
}

///////////////////////////////////////////////////////////////////////////////
std::string QuitCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return create_help_str(this,
"\n"
"  Ends the game\n"
                         );
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void SaveCommand::init(const std::vector<std::string>& args)
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
    time_t curr_time_out = time(NULL);
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
  Engine& engine = Engine::instance();
  World&  world  = engine.world();
  Player& player = engine.player();
  //PlayerAI& ai_player = engine.ai_player();

  xmlDocPtr doc = NULL;
  xmlNodePtr root_node = NULL;
  doc = xmlNewDoc(BAD_CAST "1.0");
  root_node = xmlNewNode(NULL, BAD_CAST "baal_root");
  xmlDocSetRootElement(doc, root_node);

  xmlAddChild(root_node, world.to_xml());
  xmlAddChild(root_node, player.to_xml());
  //xmlAddChild(root_node, ai_player.to_xml());

  xmlSaveFormatFileEnc(m_arg.c_str(), doc, "UTF-8", 1);
}

///////////////////////////////////////////////////////////////////////////////
std::string SaveCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return create_help_str(this,
"[filename]\n"
"  Saves the game; if no name provided, a name based on data/time will be used\n"
                     );
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void SpellCommand::init(const std::vector<std::string>& args)
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
  Engine& engine = Engine::instance();
  World&  world  = engine.world();
  Player& player = engine.player();

  // Ensure location is in-bounds
  RequireUser(world.in_bounds(m_spell_location),
              "Location " << m_spell_location << " out of bounds. " <<
              "Max row is: " << world.height() - 1 <<
              ", max col is: " << world.width() - 1);

  // Create the spell. Use an auto_ptr to ensure deletion even if a throw
  // happens. I'd rather use a reference here since spell cannot be NULL, but
  // we need to auto_ptr since verify_cast can throw exceptions.
  std::auto_ptr<const Spell> spell(
    &(SpellFactory::create_spell(m_spell_name,
                                 m_spell_level,
                                 m_spell_location))
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

///////////////////////////////////////////////////////////////////////////////
std::string SpellCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine = Engine::instance();
  const TalentTree& talents = engine.player().talents();

  std::vector<std::pair<std::string, unsigned> > castable_spells;
  talents.query_all_castable_spells(castable_spells);

  std::string help_str =
"<spell-name> <level> <row>,<col>\n"
"  Casts spell of type <spell-name> and level <level> at location <row>,<col>\n"
"  Castable spells:\n";
  for (std::vector<std::pair<std::string, unsigned> >::const_iterator
       itr = castable_spells.begin(); itr != castable_spells.end(); ++itr) {
    std::string spell_name = itr->first;
    unsigned spell_lvl     = itr->second;
    std::ostringstream out;
    out << "    " << spell_name << " : " << spell_lvl << "\n";
    help_str += out.str();
  }

  return create_help_str(this, help_str);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void LearnCommand::init(const std::vector<std::string>& args)
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
  Player& player = Engine::instance().player();
  Location dummy;

  // We need to auto_ptr since learn can throw exceptions.
  std::auto_ptr<const Spell> spell(
    &(SpellFactory::create_spell(m_spell_name,
                                 m_spell_level,
                                 dummy))
                                   );

  // Try to have the player learn this spell, this can throw
  player.learn(*spell);
}

///////////////////////////////////////////////////////////////////////////////
std::string LearnCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  Player& player = Engine::instance().player();
  const TalentTree& talents = player.talents();

  std::vector<std::pair<std::string, unsigned> > learnable_spells;
  talents.query_all_learnable_spells(learnable_spells, player);

  std::string help_str =
"<spell-name> <level>\n"
"  Player learns spell of type <spell-name> and level <level>\n"
"  Learnable spells:\n";
  for (std::vector<std::pair<std::string, unsigned> >::const_iterator
       itr = learnable_spells.begin(); itr != learnable_spells.end(); ++itr) {
    std::string spell_name = itr->first;
    unsigned spell_lvl     = itr->second;
    std::ostringstream out;
    out << "    " << spell_name << " : " << spell_lvl << "\n";
    help_str += out.str();
  }

  return create_help_str(this, help_str);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void DrawCommand::init(const std::vector<std::string>& args)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(args.size() == 1, "The draw command takes 1 argument");

  // Parse draw mode
  s_draw_mode = args[0];
}

///////////////////////////////////////////////////////////////////////////////
void DrawCommand::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  DrawMode new_draw_mode = Drawable::parse_draw_mode(s_draw_mode);
  Drawable::set_draw_mode(new_draw_mode);
  Engine::instance().interface().draw(); // redraw
}

///////////////////////////////////////////////////////////////////////////////
std::string DrawCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  std::string usage =
"<draw-mode>\n"
"  Changes how the world is drawn.\n"
"  Available draw modes:\n";
  for (DrawMode mode_itr = Drawable::FIRST; ; ++mode_itr) {
    usage += "    " + Drawable::draw_mode_to_str(mode_itr)  + "\n";
    if (mode_itr == Drawable::LAST) {
      break;
    }
  }

  return create_help_str(this, usage);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void HackCommand::init(const std::vector<std::string>& args)
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
  Player& player = Engine::instance().player();
  player.gain_exp(m_exp);
}

///////////////////////////////////////////////////////////////////////////////
std::string HackCommand::help() const
///////////////////////////////////////////////////////////////////////////////
{
  return create_help_str(this,
"<exp>\n"
"  Gives the player free arbitrary exp. This is a cheat put in for testing\n"
                         );
}

}

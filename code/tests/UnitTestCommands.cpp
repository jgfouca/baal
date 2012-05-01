#define private public

#include "Command.hpp"
#include "Configuration.hpp"
#include "Engine.hpp"
#include "Interface.hpp"
#include "InterfaceFactory.hpp"
#include "InterfaceText.hpp"
#include "Player.hpp"
#include "World.hpp"

#include <gtest/gtest.h>
#include <string>

namespace {

TEST(Command, HelpCommand)
{
  using namespace baal;

  // Set up a game engine with default settings except
  // send all text to an ostringstream
  Configuration config(InterfaceFactory::TEXT_INTERFACE +
                       InterfaceFactory::SEPARATOR +
                       InterfaceFactory::TEXT_WITH_OSTRINGSTREAM +
                       InterfaceFactory::SEPARATOR +
                       "/dev/null");
  Engine engine(config);

  InterfaceText& interface = dynamic_cast<InterfaceText&>(engine.interface());
  std::ostringstream& stream = dynamic_cast<std::ostringstream&>(interface.m_ostream);

  {
    const std::string expected =
R"(List of available commands:

help [item]
  Returns info/syntax help for an item or all commands if no argument
  Examples of valid items: command-name, spell-name, mode-name
  Aliases: h 
save [filename]
  Saves the game; if no name provided, a name based on data/time will be used
  Aliases: s 
end [num-turns]
  Ends the current turn. Optional arg to skip ahead many turns
  Aliases: n 
quit 
  Ends the game
  Aliases: q 
cast <spell-name> <row>,<col> [<level>]
  Casts spell of type <spell-name> and level <level> at location <row>,<col>
  If no level is provided, spell will be cast at player's max skill
  Aliases: c 
  Castable spells:
learn <spell-name>
  Player learns spell of type <spell-name> or increases power in that spell
  Aliases: l 
  Learnable spells:
    hot : (new)
    cold : (new)
    wind : (new)
    infect : (new)
draw <draw-mode>
  Changes how the world is drawn.
  Aliases: d 
  Available draw modes:
    CIV
    LAND
    YIELD
    MOISTURE
    GEOLOGY
    MAGMA
    TENSION
    WIND
    TEMPERATURE
    PRESSURE
    PRECIP
    DEWPOINT
    ELEVATION
    SNOWPACK
    SEASURFACETEMP
hack <exp>
  Gives the player free arbitrary exp. This is a cheat put in for testing
  Aliases: x 

)";

    HelpCommand help_command({""}, engine);
    help_command.apply();
    EXPECT_EQ(expected, stream.str());

    stream.clear();
    stream.str("");
  }

  {
    const std::string expected =
R"(cast <spell-name> <row>,<col> [<level>]
  Casts spell of type <spell-name> and level <level> at location <row>,<col>
  If no level is provided, spell will be cast at player's max skill
  Aliases: c 
  Castable spells:

)";

    HelpCommand help_command({"cast"}, engine);
    help_command.apply();
    EXPECT_EQ(expected, stream.str());

    stream.clear();
    stream.str("");
  }

  {
    const std::string expected =
R"(Description of fire spell:
TODO
Player has skill level 0 in this spell
)";

    HelpCommand help_command({"fire"}, engine);
    help_command.apply();
    EXPECT_EQ(expected, stream.str());

    stream.clear();
    stream.str("");
  }

  {
    const std::string expected =
R"(Description of draw-mode: temperature
Draws the current average (not high or low) temperature in degrees farenheit.
)";

    HelpCommand help_command({"temperature"}, engine);
    help_command.apply();
    EXPECT_EQ(expected, stream.str());

    stream.clear();
    stream.str("");
  }
}

TEST(Command, SpellCommands)
{
  using namespace baal;

  // Set up a game engine with default settings except
  // send all text to an ostringstream
  Configuration config(InterfaceFactory::TEXT_INTERFACE +
                       InterfaceFactory::SEPARATOR +
                       InterfaceFactory::TEXT_WITH_OSTRINGSTREAM +
                       InterfaceFactory::SEPARATOR +
                       "/dev/null");
  Engine engine(config);

  {
    LearnCommand command({"hot"}, engine);
    command.apply();
    EXPECT_TRUE(engine.player().talents().has("hot"));
  }

  {
    int orig_temp =
      engine.world().get_tile(Location(1, 1)).atmosphere().temperature();
    SpellCommand command({"hot", "1,1"}, engine);
    command.apply();
    int new_temp =
      engine.world().get_tile(Location(1, 1)).atmosphere().temperature();
    EXPECT_GT(new_temp, orig_temp);
  }

}

}

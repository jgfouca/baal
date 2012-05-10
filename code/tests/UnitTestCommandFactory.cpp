#define private public

#include "Command.hpp"
#include "CommandFactory.hpp"
#include "Configuration.hpp"
#include "Engine.hpp"

#include <gtest/gtest.h>
#include <string>
#include <memory>

namespace {

TEST(CommandFactory, test_initialization)
{
  using namespace baal;

  const CommandFactory& cf = CommandFactory::instance();

  std::vector<std::string> expected_commands =
    {"help", "save", "end", "quit", "cast", "learn", "draw", "hack"};
  EXPECT_EQ(expected_commands, cf.commands());

  std::map<std::string, std::string> expected_aliases =
    {
      {"h", "help"},
      {"s", "save"},
      {"n", "end"},
      {"q", "quit"},
      {"c", "cast"},
      {"l", "learn"},
      {"d", "draw"},
      {"x", "hack"}
    };
  EXPECT_EQ(expected_aliases, cf.m_aliases);
}

TEST(CommandFactory, test_parsing)
{
  using namespace baal;

  const CommandFactory& cf = CommandFactory::instance();
  auto engine = create_engine();

  {
    auto command = cf.parse_command("h", *engine);
    auto help_cmd = std::dynamic_pointer_cast<const HelpCommand>(command);
    EXPECT_TRUE(bool(help_cmd));
  }

  {
    auto command = cf.parse_command("cast fire 1,1 1", *engine);
    auto spell_cmd = std::dynamic_pointer_cast<const SpellCommand>(command);
    EXPECT_TRUE(bool(spell_cmd));
  }

  {
    auto command = cf.parse_command("c fire 1,1 1", *engine);
    auto spell_cmd = std::dynamic_pointer_cast<const SpellCommand>(command);
    EXPECT_TRUE(bool(spell_cmd));
  }

  {
    EXPECT_THROW(cf.parse_command("asdadd", *engine), UserError);
  }
}

}

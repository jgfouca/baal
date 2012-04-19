#define private public

#include "Command.hpp"
#include "Configuration.hpp"
#include "Engine.hpp"
#include "Interface.hpp"
#include "InterfaceFactory.hpp"

#include <gtest/gtest.h>

namespace {

TEST(Command, HelpCommand)
{
  using namespace baal;

  // Set up a game engine with default settings except
  // send all text to /dev/null
  Configuration config;
  Engine engine(config);
}

}

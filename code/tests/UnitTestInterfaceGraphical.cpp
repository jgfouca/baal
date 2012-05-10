#include "InterfaceGraphical.hpp"
#include "InterfaceFactory.hpp"
#include "Engine.hpp"
#include "Configuration.hpp"

#include <gtest/gtest.h>

namespace {

TEST(InterfaceGraphical, basic)
{
  using namespace baal;

  Configuration config(InterfaceFactory::GRAPHICAL_INTERFACE);
  EXPECT_THROW(create_engine(config), UserError);
}

}

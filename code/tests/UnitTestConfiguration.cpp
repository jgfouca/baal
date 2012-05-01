#include "Configuration.hpp"

#include <gtest/gtest.h>

namespace {

TEST(Configuration, Basic)
{
  using namespace baal;
  static const std::string EMPTY, ONE="one", TWO="two", THREE="three";
  {
    Configuration config;
    EXPECT_EQ(EMPTY, config.get_interface_config());
    EXPECT_EQ(EMPTY, config.get_world_config());
    EXPECT_EQ(EMPTY, config.get_player_config());
  }

  {
    Configuration config(ONE);
    EXPECT_EQ(ONE, config.get_interface_config());
    EXPECT_EQ(EMPTY, config.get_world_config());
    EXPECT_EQ(EMPTY, config.get_player_config());
  }

  {
    Configuration config(ONE, TWO);
    EXPECT_EQ(ONE, config.get_interface_config());
    EXPECT_EQ(TWO, config.get_world_config());
    EXPECT_EQ(EMPTY, config.get_player_config());
  }

  {
    Configuration config(ONE, TWO, THREE);
    EXPECT_EQ(ONE, config.get_interface_config());
    EXPECT_EQ(TWO, config.get_world_config());
    EXPECT_EQ(THREE, config.get_player_config());
  }
}

}

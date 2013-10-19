#include "Player.hpp"
#include "Engine.hpp"

#include <gtest/gtest.h>

namespace {

TEST(Player, PlayerBasics)
{
  //
  // Test simple player queries
  //

  auto engine = baal::create_engine();
  baal::Player& player = engine->player();

  EXPECT_EQ(player.max_mana(), player.mana());

}


}

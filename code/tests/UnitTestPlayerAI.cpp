#include "PlayerAI.hpp"
#include "Engine.hpp"
#include "Spell.hpp"
#include "SpellFactory.hpp"

#include <gtest/gtest.h>

namespace {

TEST(PlayerAI, PlayerAIBasics)
{
  //
  // Test simple player queries
  //

  auto engine = baal::create_engine();
  baal::PlayerAI& ai = engine->ai_player();

  EXPECT_EQ(ai.tech_level(), 1u);

  while (ai.tech_level() == 1) {
    const unsigned tech_points = ai.tech_points();
    ai.cycle_turn();
    EXPECT_GT(ai.tech_points(), tech_points);
  }

  EXPECT_EQ(ai.tech_level(), 2u);
}


}

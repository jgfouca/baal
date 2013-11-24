#include "Player.hpp"
#include "Engine.hpp"
#include "Spell.hpp"
#include "SpellFactory.hpp"

#include <gtest/gtest.h>

namespace {

TEST(Player, PlayerBasics)
{
  //
  // Test simple player queries
  //

  auto engine = baal::create_engine();
  baal::Player& player = engine->player();
  auto spell = baal::SpellFactory::create_spell(baal::Hot::NAME, *engine);

  EXPECT_EQ(player.max_mana(), player.mana());
  EXPECT_EQ(player.level(), 1u);

  player.learn(spell->name());
  player.verify_cast(*spell);

  while(player.mana() >= spell->cost()) {
    player.verify_cast(*spell);
    player.cast(*spell);
  }
  EXPECT_THROW(player.verify_cast(*spell), baal::UserError);

  for (int i = 0; i < 100; ++i) {
    player.cycle_turn();
  }
  EXPECT_EQ(player.mana(), player.max_mana());

  player.verify_cast(*spell);

  auto cold = baal::SpellFactory::create_spell(baal::Cold::NAME, *engine);
  EXPECT_THROW(player.verify_cast(*cold), baal::UserError);
  EXPECT_THROW(player.learn(cold->name()), baal::UserError);

  for (int i = 0; i < 3; ++i) {
    player.gain_exp(player.next_level_cost());
  }
  EXPECT_EQ(player.level(), 4u);
  EXPECT_EQ(player.exp(), 0u);

  for (int i = 0; i < 3; ++i) {
    player.gain_exp(player.next_level_cost() / 4);
  }
  EXPECT_EQ(player.level(), 4u);

  auto cold_2 = baal::SpellFactory::create_spell(baal::Cold::NAME, *engine, 2 /*level*/);
  player.learn(cold->name());
  EXPECT_THROW(player.verify_cast(*cold_2), baal::UserError);
  player.verify_cast(*cold);
  player.learn(cold->name());
  player.verify_cast(*cold_2);
}


}

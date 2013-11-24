#include "Player.hpp"
#include "Engine.hpp"
#include "Spell.hpp"
#include "SpellFactory.hpp"
#include "TalentTree.hpp"

#include <gtest/gtest.h>

namespace {

TEST(TalentTree, Basic)
{
  using namespace baal;

  auto engine = create_engine();
  Player& player = engine->player();
  TalentTree talents(player);

  const std::string tier1_spell_name = "hot";
  const std::string another_tier1_spell_name = "cold";
  const std::string tier2_spell_name = "fire";
  const std::string high_tier_spell_name = "quake";
  const std::string not_a_spell = "lol";

  // Get player to level 10
  for (int i = 0; i < 9; ++i) {
    const unsigned exp_needed = player.next_level_cost();
    player.gain_exp(exp_needed);
  }

  EXPECT_EQ(player.level(), 10u);

  // Should not be able to learn fire 1 (don't know hot 1)
  EXPECT_THROW(talents.add(tier2_spell_name), UserError);

  // Should not be able to learn quake 1 (not high enough level)
  EXPECT_THROW(talents.add(high_tier_spell_name), UserError);

  // Should not be able to learn an invalid spell
  EXPECT_THROW(talents.add(not_a_spell), UserError);

  // Learn hot up to max
  for (unsigned i = 0; i < TalentTree::MAX_SPELL_LEVEL; ++i) {
    EXPECT_EQ(talents.num_learned(), i);
    EXPECT_EQ(talents.spell_skill(tier1_spell_name), i);
    EXPECT_FALSE(talents.has(tier1_spell_name, i+1));
    talents.add(tier1_spell_name);
    EXPECT_EQ(talents.spell_skill(tier1_spell_name), i+1);
    EXPECT_TRUE(talents.has(tier1_spell_name, i+1));
    EXPECT_EQ(talents.num_learned(), i+1);
  }

  // Should not be able to level up hot spell any higher
  EXPECT_THROW(talents.add(tier1_spell_name), UserError);

  // Should be able to learn fire now
  for (unsigned i = 0; i < TalentTree::MAX_SPELL_LEVEL; ++i) {
    talents.add(tier2_spell_name);
    EXPECT_EQ(talents.num_learned(), i+1 + TalentTree::MAX_SPELL_LEVEL);
  }

  // Should not be able to learn a different tier1 spell, out of points
  EXPECT_THROW(talents.add(another_tier1_spell_name), UserError);

  // Check that we know the spells  we've learned
  TalentTree::query_return_type learned = talents.query_all_castable_spells();
  EXPECT_EQ(learned.size(), 2u);
  for (auto spell_spec : learned) {
    EXPECT_TRUE(talents.has(spell_spec.first, spell_spec.second));
  }
}


}

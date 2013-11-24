#include "Player.hpp"
#include "Engine.hpp"
#include "Spell.hpp"
#include "SpellFactory.hpp"

#include <gtest/gtest.h>

namespace {

TEST(SpellFactory, Basic)
{
  baal::Location loc(2,4);
  auto engine = baal::create_engine();
  auto hot    = baal::SpellFactory::create_spell(baal::Hot::NAME, *engine, 5, loc);

  EXPECT_EQ(hot->name(), baal::Hot::NAME);
  EXPECT_EQ(hot->level(), 5u);
  EXPECT_EQ(hot->location(), loc);

  EXPECT_THROW(baal::SpellFactory::create_spell("does not exist", *engine), baal::UserError);
}


}

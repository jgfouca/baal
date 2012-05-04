#include "Geology.hpp"

#include <gtest/gtest.h>

namespace {

TEST(Geology, is_geological)
{
  using namespace baal;

  EXPECT_TRUE(Geology::is_geological(MAGMA));
  EXPECT_FALSE(Geology::is_geological(DEWPOINT));
}

TEST(Geology, Subducting)
{
  using namespace baal;

  const float plate_movement = 3.0;

  Subducting geology(plate_movement);

  EXPECT_GT(geology.tension_buildup(), 0.0);
  EXPECT_GT(geology.magma_buildup(),   0.0);

  geology.cycle_turn();
  geology.cycle_turn();

  float tension = geology.tension();
  float magma   = geology.magma();

  geology.cycle_turn();
  geology.cycle_turn();

  EXPECT_LT(tension, geology.tension());
  EXPECT_LT(magma  , geology.magma());

  for (unsigned i = 0; i < 1000; ++i) {
    geology.cycle_turn();
  }

  EXPECT_LT(geology.tension(), 1.0);
  EXPECT_LT(geology.magma(),   1.0);
}

TEST(Geology, Transform)
{
  using namespace baal;

  const float plate_movement = 3.0;

  Transform geology(plate_movement);

  EXPECT_GT(geology.tension_buildup(), 0.0);
  EXPECT_EQ(geology.magma_buildup(),   0.0);

  float tension = geology.tension();
  float magma   = geology.magma();

  geology.cycle_turn();
  geology.cycle_turn();

  EXPECT_LT(tension, geology.tension());
  EXPECT_EQ(magma  , geology.magma());
}

TEST(Geology, Inactive)
{
  using namespace baal;

  Inactive geology;

  EXPECT_EQ(geology.tension_buildup(), 0.0);
  EXPECT_EQ(geology.magma_buildup(),   0.0);

  float tension = geology.tension();
  float magma   = geology.magma();

  for (unsigned i = 0; i < 1000; ++i) {
    geology.cycle_turn();
  }

  EXPECT_EQ(tension, geology.tension());
  EXPECT_EQ(magma  , geology.magma());
}

}

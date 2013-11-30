#include "Weather.hpp"
#include "Engine.hpp"
#include "World.hpp"

#include <gtest/gtest.h>
#include <sstream>

namespace {

TEST(World, basic)
{
  using namespace baal;

  auto engine = baal::create_engine();
  World& world = engine->world();

  const unsigned width = world.width();
  const unsigned height = world.height();

  EXPECT_FALSE(world.in_bounds(Location(width, height)));
  EXPECT_FALSE(world.in_bounds(Location(width, height - 1)));
  EXPECT_FALSE(world.in_bounds(Location(width - 1, height)));
  EXPECT_TRUE(world.in_bounds(Location(width - 1, height - 1)));

  EXPECT_EQ(world.get_tile(Location(4, 2)).location(), Location(4, 2));

  const std::vector<City*>& cities = world.cities();
  EXPECT_EQ(cities.size(), 1u);

  world.remove_city(*cities[0]);

  EXPECT_EQ(world.cities().size(), 0u);
}


}
